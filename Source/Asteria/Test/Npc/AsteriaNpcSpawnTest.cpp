// Fill out your copyright notice in the Description page of Project Settings.

// 게임 빌드에는 CQTest가 없으므로 에디터 빌드에서만 컴파일한다.
#if WITH_EDITOR

#include "Test/Base/AsteriaNetworkComponent.h"

#if ENABLE_PIE_NETWORK_TEST

#include "EngineUtils.h"
#include "NPC/AsteriaNpc.h"

// 앞선 테스트가 로드한 환경에서 서버와 클라이언트 각각에 스포너가 스폰한 Npc가 1개 있는지 확인하는 테스트.
NETWORK_TEST_CLASS(AsteriaNpcSpawn, "Asteria.01_Npc.Spawn")
{
	FAsteriaNetworkComponent Network{ TestRunner, TestCommandBuilder, bInitializing, false };

	// Npc 스폰 대기 제한 시간.
	const FTimespan NpcWaitTimeout = FTimespan::FromSeconds(10);

	// Npc 대기를 시작한 시각. 서버와 클라이언트 대기마다 새로 잡는다.
	FDateTime NpcWaitStart;

	static int32 CountNpcs(UWorld* World)
	{
		int32 Count = 0;
		for (TActorIterator<AAsteriaNpc> It(World); It; ++It)
		{
			++Count;
		}
		return Count;
	}

	// World에 Npc가 1개 이상 보이면 true. 제한 시간이 지나면 한 줄 남기고 true를 돌려 확인을 진행한다.
	bool AwaitNpcSpawned(const TCHAR* Role, UWorld* World)
	{
		if (CountNpcs(World) >= 1)
		{
			FAsteriaNetworkComponent::Log(FString::Printf(TEXT("[%s] Npc 스폰 대기 완료"), Role));
			return true;
		}
		if (FDateTime::UtcNow() - NpcWaitStart >= NpcWaitTimeout)
		{
			FAsteriaNetworkComponent::Log(FString::Printf(TEXT("[%s] Npc 스폰 대기 시간 초과 %d초"), Role, static_cast<int32>(NpcWaitTimeout.GetTotalSeconds())));
			return true;
		}
		return false;
	}

	// World의 Npc 수가 1인지 확인한다.
	void CheckNpcSpawned(const TCHAR* Role, UWorld* World)
	{
		const int32 NpcCount = CountNpcs(World);
		Network.LogCheck(Role, TEXT("Npc"), TEXT("1"), FString::FromInt(NpcCount), NpcCount == 1);
	}

	TEST_METHOD(ServerAndClient_SpawnOneNpcFromSpawner)
	{
		Network.ResumeGameUntilTestEnd();

		const FTimespan EngineTimeout = NpcWaitTimeout * 2.0;

		Network
			.ThenServer(TEXT("Start Server Npc Wait"), [this](FBasePIENetworkComponentState& State)
			{
				FAsteriaNetworkComponent::Log(TEXT("[서버] Npc 스폰 대기 시작"));
				NpcWaitStart = FDateTime::UtcNow();
			})
			// Npc 대기는 스스로 NpcWaitTimeout 뒤에 끝내므로 엔진 타임아웃은 그보다 넉넉하게 둔다.
			.UntilServer(TEXT("Await Server Npc"), [this](FBasePIENetworkComponentState& State)
			{
				return AwaitNpcSpawned(TEXT("서버"), State.World);
			}, EngineTimeout)
			.ThenServer(TEXT("Check Server Npc"), [this](FBasePIENetworkComponentState& State)
			{
				CheckNpcSpawned(TEXT("서버"), State.World);
			})
			.ThenClients(TEXT("Start Client Npc Wait"), [this](FBasePIENetworkComponentState& State)
			{
				FAsteriaNetworkComponent::Log(TEXT("[클라이언트] Npc 스폰 대기 시작"));
				NpcWaitStart = FDateTime::UtcNow();
			})
			.UntilClients(TEXT("Await Client Npc"), [this](FBasePIENetworkComponentState& State)
			{
				return AwaitNpcSpawned(TEXT("클라이언트"), State.World);
			}, EngineTimeout)
			.ThenClients(TEXT("Check Client Npc"), [this](FBasePIENetworkComponentState& State)
			{
				CheckNpcSpawned(TEXT("클라이언트"), State.World);
			});
	}
};

#endif // ENABLE_PIE_NETWORK_TEST

#endif // WITH_EDITOR
