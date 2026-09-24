// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// 게임 빌드에는 CQTest가 없으므로 에디터 빌드에서만 컴파일한다.
#if WITH_EDITOR

#include "Components/PIENetworkComponent.h"

#if ENABLE_PIE_NETWORK_TEST

#include "GameMapsSettings.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

/**
 * 서버(리슨)와 클라이언트 1개를 띄우고 에디터에 열려 있는 현재 맵을 싣는 네트워크 테스트 공통 기반.
 * 테스트 클래스가 멤버로 두고 ThenServer, ThenClient 등으로 단계를 이어 붙인다.
 *
 * 엔진 기본(FBasePIENetworkComponent)은 빈 새 맵을 만들어 띄우므로, 부모를 초기화 모드로 불러
 * 그 준비 단계를 막고 "Create New Map"만 뺀 같은 순서를 여기서 다시 등록한다.
 *
 * bLoadEnvironment가 false면 PIE를 다시 띄우지 않고 앞선 테스트가 띄운 PIE 세션을 그대로 쓴다.
 * 어느 쪽이든 테스트가 끝나도 PIE는 끄지 않고 에디터 설정만 되돌려 다음 테스트가 같은 환경을 쓰게 한다.
 */
class FAsteriaNetworkComponent : public FPIENetworkComponent<>
{
public:
	FAsteriaNetworkComponent(FAutomationTestBase* InTestRunner, FTestCommandBuilder& InCommandBuilder, bool IsInitializing, bool bLoadEnvironment = true)
		: FPIENetworkComponent(InTestRunner, InCommandBuilder, true)
	{
		if (IsInitializing)
		{
			return;
		}

		UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
		MapName = EditorWorld->GetMapName();

		// Build가 World Settings의 게임 모드를 덮어쓰므로 그 전에 구한다.
		TSubclassOf<AGameModeBase> MapGameMode = EditorWorld->GetWorldSettings()->DefaultGameMode;
		if (MapGameMode == nullptr)
		{
			MapGameMode = FSoftClassPath(UGameMapsSettings::GetGlobalDefaultGameMode()).TryLoadClass<AGameModeBase>();
		}
		PawnClass = MapGameMode->GetDefaultObject<AGameModeBase>()->DefaultPawnClass;

		FNetworkComponentBuilder<>()
			.AsListenServer()
			.WithClients(1)
			.WithGameMode(MapGameMode)
			.Build(*this);

		FTimespan TimeoutValue = MakeTimeout(CQTest::DefaultTimeout);

		// 환경을 다시 로드하지 않으면 떠 있는 PIE 세션의 월드와 접속 정보만 확보한다.
		if (!bLoadEnvironment)
		{
			CommandBuilder
				->Until(TEXT("Set Worlds"), [this]() { return SetWorlds(); }, TimeoutValue)
				.Then(TEXT("Connect Clients to Server"), [this]() { ConnectClientsToServer(); })
				.Until(TEXT("Await Clients Ready"), [this]() { return AwaitClientsReady(); }, TimeoutValue)
				.Then(TEXT("Log Reused"), [this]() { LogReused(); })
				.OnTearDown(TEXT("Restore Editor Settings"), [this]() { StateRestorer.Restore(); });
			return;
		}

		CommandBuilder
			->Do(TEXT("Stop PIE"), [this]() { StopPie(); })
			.Then(TEXT("Start PIE"), [this]() { StartPie(); })
			.Until(TEXT("Set Worlds"), [this]() { return SetWorlds(); }, TimeoutValue)
			.Then(TEXT("Setup Packet Settings"), [this]() { SetPacketSettings(); })
			.Then(TEXT("Connect Clients to Server"), [this]() { ConnectClientsToServer(); })
			.Until(TEXT("Await Clients Ready"), [this]() { return AwaitClientsReady(); }, TimeoutValue)
			.Then(TEXT("Log Loaded"), [this]() { LogLoaded(); })
			// 캐릭터 대기는 스스로 TimeoutValue 뒤에 끝내므로 엔진 타임아웃은 그보다 넉넉하게 둔다.
			.Until(TEXT("Await Characters Loaded"), [this, TimeoutValue]() { return AwaitCharactersLoaded(TimeoutValue); }, TimeoutValue * 2.0)
			.Then(TEXT("Log Character Loaded"), [this]() { LogCharacterLoaded(); })
			// PIE는 끄지 않고 게임 모드와 게임 인스턴스 설정만 되돌린다.
			.OnTearDown(TEXT("Restore Editor Settings"), [this]() { StateRestorer.Restore(); });
	}

	// 테스트 로그 한 줄을 언리얼 로그와 Saved/TestLogs/AsteriaTest.log에 남긴다. 파일은 실행마다 새로 쓴다.
	static void Log(const FString& Line)
	{
		UE_LOG(LogTemp, Display, TEXT("%s"), *Line);

		static bool bFirstWrite = true;
		const FString Path = FPaths::ProjectSavedDir() / TEXT("TestLogs") / TEXT("AsteriaTest.log");
		IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
		FFileHelper::SaveStringToFile(Line + LINE_TERMINATOR, *Path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM,
			&IFileManager::Get(), bFirstWrite ? FILEWRITE_None : FILEWRITE_Append);
		bFirstWrite = false;
	}

	// 확인 하나를 대상, 기대 값, 실제 값, 통과 여부 한 줄로 기록한다. 실패면 테스트 오류.
	void LogCheck(const TCHAR* Role, const FString& Target, const FString& Expected, const FString& Actual, bool bPassed)
	{
		Log(FString::Printf(TEXT("[%s] %s 기대 %s == %s 실제 %s (%s)"), Role, *Target, *Expected, *Target, *Actual, bPassed ? TEXT("통과") : TEXT("실패")));
		if (!bPassed)
		{
			TestRunner->AddError(FString::Printf(TEXT("[%s] %s 확인 실패, 기대 %s, 실제 %s"), Role, *Target, *Expected, *Actual));
		}
	}

protected:
	// 테스트 시작 시점에 에디터에 열려 있던 맵 이름.
	FString MapName;

	// 현재 맵 게임 모드의 기본 캐릭터 클래스. 캐릭터 로드 검증의 기준.
	TSubclassOf<APawn> PawnClass;

	// 캐릭터 대기를 시작한 시각. 대기 시간 초과 판단용.
	FDateTime CharacterWaitStart;

private:
	// 서버와 클라이언트가 앞선 테스트가 로드한 환경을 쓰고 있음을 각각 한 줄로 기록한다.
	void LogReused()
	{
		Log(TEXT("[서버] 로드된 환경 사용"));
		for (int32 Index = 0; Index < ClientStates.Num(); ++Index)
		{
			Log(TEXT("[클라이언트] 로드된 환경 사용"));
		}
	}

	// 서버와 클라이언트가 MapName 맵을 실었는지 각각 한 줄로 기록한다. 다르면 테스트 오류.
	void LogLoaded()
	{
		auto LogWorld = [this](const TCHAR* Role, const UWorld* World)
		{
			const FString LoadedMapName = UWorld::RemovePIEPrefix(World->GetMapName());
			const bool bLoaded = LoadedMapName == MapName;
			Log(FString::Printf(TEXT("[%s] %s 맵 로드 %s"), Role, *MapName, bLoaded ? TEXT("성공") : TEXT("실패")));
			if (!bLoaded)
			{
				TestRunner->AddError(FString::Printf(TEXT("[%s] 기대한 맵: %s, 실제 맵: %s"), Role, *MapName, *LoadedMapName));
			}
		};

		LogWorld(TEXT("서버"), ServerState->World);
		for (const TUniquePtr<FBasePIENetworkComponentState>& ClientState : ClientStates)
		{
			LogWorld(TEXT("클라이언트"), ClientState->World);
		}
	}

	// World의 플레이어 컨트롤러 중 PawnClass 캐릭터를 조종하고 있는 수.
	int32 CountLoadedCharacters(const UWorld* World) const
	{
		int32 Count = 0;
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			const APawn* Pawn = It->IsValid() ? (*It)->GetPawn() : nullptr;
			if (Pawn != nullptr && Pawn->IsA(PawnClass))
			{
				++Count;
			}
		}
		return Count;
	}

	// 서버는 호스트와 모든 클라이언트, 각 클라이언트는 자기 캐릭터 1개가 준비되면 true.
	// Timeout이 지나도 true를 돌려 테스트를 멈추지 않고 LogCharacterLoaded에서 실패를 기록하게 한다.
	bool AwaitCharactersLoaded(const FTimespan& Timeout)
	{
		if (CharacterWaitStart == FDateTime())
		{
			CharacterWaitStart = FDateTime::UtcNow();
		}
		if (FDateTime::UtcNow() - CharacterWaitStart >= Timeout)
		{
			return true;
		}

		if (CountLoadedCharacters(ServerState->World) < 1 + ClientStates.Num())
		{
			return false;
		}
		for (const TUniquePtr<FBasePIENetworkComponentState>& ClientState : ClientStates)
		{
			if (CountLoadedCharacters(ClientState->World) < 1)
			{
				return false;
			}
		}
		return true;
	}

	// 서버와 클라이언트가 플레이어 캐릭터를 실었는지 각각 한 줄로 기록한다. 모자라면 테스트 오류.
	void LogCharacterLoaded()
	{
		auto LogWorld = [this](const TCHAR* Role, const UWorld* World, int32 Expected)
		{
			const int32 Actual = CountLoadedCharacters(World);
			const bool bLoaded = Actual >= Expected;
			Log(FString::Printf(TEXT("[%s] 플레이어 캐릭터 로드 %s"), Role, bLoaded ? TEXT("성공") : TEXT("실패")));
			if (!bLoaded)
			{
				TestRunner->AddError(FString::Printf(TEXT("[%s] 기대한 캐릭터 수: %d, 실제 캐릭터 수: %d"), Role, Expected, Actual));
			}
		};

		LogWorld(TEXT("서버"), ServerState->World, 1 + ClientStates.Num());
		for (const TUniquePtr<FBasePIENetworkComponentState>& ClientState : ClientStates)
		{
			LogWorld(TEXT("클라이언트"), ClientState->World, 1);
		}
	}
};

#endif // ENABLE_PIE_NETWORK_TEST

#endif // WITH_EDITOR
