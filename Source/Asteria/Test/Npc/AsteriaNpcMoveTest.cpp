// Fill out your copyright notice in the Description page of Project Settings.

// 게임 빌드에는 CQTest가 없으므로 에디터 빌드에서만 컴파일한다.
#if WITH_EDITOR

#include "Test/Base/AsteriaNetworkComponent.h"

#if ENABLE_PIE_NETWORK_TEST

#include "EngineUtils.h"
#include "Components/CapsuleComponent.h"
#include "Actors/ApproachPointActor.h"
#include "NPC/AsteriaNpc.h"

// 앞선 테스트가 로드한 환경에서 서버와 클라이언트 각각 Npc가 QuestBoard 접근 지점 도착 반경 이내에 도착하는지 확인하는 테스트.
NETWORK_TEST_CLASS(AsteriaNpcMove, "Asteria.02_Npc.MoveToQuestBoard")
{
	FAsteriaNetworkComponent Network{ TestRunner, TestCommandBuilder, bInitializing, false };

	// Npc QuestBoard 도착 대기 제한 시간.
	const FTimespan ArriveWaitTimeout = FTimespan::FromSeconds(30);

	// 한 월드의 위치 추적 상태.
	struct FTrack
	{
		UWorld* World = nullptr;
		// 이 월드의 도착 반경. 추적 시작 때 한 번 읽는다.
		int32 Radius = -1;
		// 추적 중 가장 가까웠던 거리. 아직 읽은 거리가 없으면 -1.
		int32 MinDistance = -1;
		bool bArrived = false;
	};

	FTrack ServerTrack;
	FTrack ClientTrack;

	// 도착 대기를 시작한 시각.
	FDateTime ArriveWaitStart;

	// World의 첫 Npc와 QuestBoard 접근 지점들 사이의 가장 짧은 수평 거리. Npc나 접근 지점이 없으면 -1.
	static int32 DistanceToQuestBoard(UWorld* World)
	{
		TActorIterator<AAsteriaNpc> NpcIt(World);
		if (!NpcIt)
		{
			return -1;
		}
		const FVector NpcLocation = NpcIt->GetActorLocation();

		double BestDistance = -1.0;
		for (TActorIterator<AApproachPointActor> It(World); It; ++It)
		{
			if (It->PointType != EApproachPointType::QuestBoard)
			{
				continue;
			}
			const double Distance = FVector::Dist2D(NpcLocation, It->GetActorLocation());
			if (BestDistance < 0.0 || Distance < BestDistance)
			{
				BestDistance = Distance;
			}
		}
		return BestDistance < 0.0 ? -1 : FMath::RoundToInt(BestDistance);
	}

	// World의 첫 Npc 캡슐 반경으로 계산한 도착 반경. Npc가 없으면 -1.
	// MoveToActor 기본 호출(bStopOnOverlap true)은 몸통 가장자리 기준으로 멈추므로 50에 캡슐 반경 × 1.1을 더한다.
	// 1.1은 엔진 UPathFollowingComponent::MinAgentRadiusPct 기본값. protected이고 리플렉션 대상이 아니라 읽을 수 없어 적어 둔다.
	static int32 ArriveRadius(UWorld* World)
	{
		TActorIterator<AAsteriaNpc> NpcIt(World);
		if (!NpcIt)
		{
			return -1;
		}
		return FMath::RoundToInt(50.0f + NpcIt->GetCapsuleComponent()->GetScaledCapsuleRadius() * 1.1f);
	}

	// 이번 프레임 거리로 가장 가까웠던 거리를 갱신하고, 반경 안에 들어오면 도착으로 추적을 끝낸다.
	static void UpdateTrack(FTrack& Track)
	{
		if (Track.bArrived)
		{
			return;
		}
		const int32 Distance = DistanceToQuestBoard(Track.World);
		if (Distance >= 0 && (Track.MinDistance < 0 || Distance < Track.MinDistance))
		{
			Track.MinDistance = Distance;
		}
		if (Track.MinDistance >= 0 && Track.MinDistance <= Track.Radius)
		{
			Track.bArrived = true;
		}
	}

	// 두 월드가 모두 도착하면 true. 제한 시간이 지나면 도착하지 못한 쪽마다 한 줄 남기고 true를 돌려 확인을 진행한다.
	bool AwaitArrived()
	{
		UpdateTrack(ServerTrack);
		UpdateTrack(ClientTrack);
		if (ServerTrack.bArrived && ClientTrack.bArrived)
		{
			return true;
		}
		if (FDateTime::UtcNow() - ArriveWaitStart >= ArriveWaitTimeout)
		{
			const int32 TimeoutSeconds = static_cast<int32>(ArriveWaitTimeout.GetTotalSeconds());
			if (!ServerTrack.bArrived)
			{
				FAsteriaNetworkComponent::Log(FString::Printf(TEXT("[서버] Npc QuestBoard 도착 대기 시간 초과 %d초"), TimeoutSeconds));
			}
			if (!ClientTrack.bArrived)
			{
				FAsteriaNetworkComponent::Log(FString::Printf(TEXT("[클라이언트] Npc QuestBoard 도착 대기 시간 초과 %d초"), TimeoutSeconds));
			}
			return true;
		}
		return false;
	}

	// 추적 중 가장 가까웠던 거리가 그 월드의 도착 반경 이하인지 확인한다.
	void CheckArrived(const TCHAR* Role, const FTrack& Track)
	{
		Network.LogCheck(Role, TEXT("QuestBoard 거리"), FString::Printf(TEXT("%d 이하"), Track.Radius),
			FString::FromInt(Track.MinDistance), Track.MinDistance >= 0 && Track.MinDistance <= Track.Radius);
	}

	TEST_METHOD(ServerAndClient_NpcMovesToQuestBoard)
	{
		const FTimespan EngineTimeout = ArriveWaitTimeout * 2.0;

		Network
			.ThenServer(TEXT("Start Server Track"), [this](FBasePIENetworkComponentState& State)
			{
				ServerTrack = FTrack{ State.World, ArriveRadius(State.World) };
				ArriveWaitStart = FDateTime::UtcNow();
				FAsteriaNetworkComponent::Log(TEXT("[서버] Npc QuestBoard 위치 추적 시작"));
			})
			.ThenClients(TEXT("Start Client Track"), [this](FBasePIENetworkComponentState& State)
			{
				ClientTrack = FTrack{ State.World, ArriveRadius(State.World) };
				FAsteriaNetworkComponent::Log(TEXT("[클라이언트] Npc QuestBoard 위치 추적 시작"));
			})
			// 도착 대기는 스스로 ArriveWaitTimeout 뒤에 끝내므로 엔진 타임아웃은 그보다 넉넉하게 둔다.
			.Until(TEXT("Await Npc Arrived"), [this]()
			{
				return AwaitArrived();
			}, EngineTimeout)
			.Then(TEXT("Check Npc Arrived"), [this]()
			{
				CheckArrived(TEXT("서버"), ServerTrack);
				CheckArrived(TEXT("클라이언트"), ClientTrack);
			});
	}
};

#endif // ENABLE_PIE_NETWORK_TEST

#endif // WITH_EDITOR
