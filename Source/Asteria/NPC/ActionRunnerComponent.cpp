// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActionRunnerComponent.h"

#include "NpcActionSet.h"
#include "Place/PlaceComponent.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

UActionRunnerComponent::UActionRunnerComponent()
{
	// 이벤트 구동(브레인이 RunSet, FinishStep이 Advance) — 틱 불필요.
	PrimaryComponentTick.bCanEverTick = false;
}

void UActionRunnerComponent::RunSet(UNpcActionSet* Set, const FVector& InTarget)
{
	Sequence = (Set != nullptr) ? Set->Steps : TArray<FActionStep>();
	Target = InTarget;
	Index = 0;
	TargetPlace = nullptr; // 새 행동 → 건물 다시 확정.

	// 최초 실행은 브레인 틱(트리 밖)에서 오니 직접 호출 OK.
	RunCurrent();
}

void UActionRunnerComponent::Advance()
{
	++Index;

	// FinishStep(지금 도는 원자 BT 태스크) 안에서 불림 → 여기서 바로 트리를
	// 교체하면 자기 트리를 실행 중 교체하는 재진입. 다음 틱으로 미룬다.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(this, &UActionRunnerComponent::RunCurrent);
	}
}

void UActionRunnerComponent::RunCurrent()
{
	AAIController* AI = Cast<AAIController>(GetOwner());
	if (AI == nullptr)
	{
		return;
	}

	if (!Sequence.IsValidIndex(Index))
	{
		Stop();
		return;
	}

	const FActionStep& Step = Sequence[Index];
	if (Step.BT == nullptr)
	{
		Advance(); // null 원자 건너뜀.
		return;
	}

	// 이동 스텝(Points 있음)이면 목적지 건물을 확정한다. 최초 이동에서 1회 스캔·캐시,
	// 남은 이동 스텝은 같은 건물 재사용(입구·좌석이 같은 건물이 되도록).
	const bool bMove = Step.Point != EPlacePoint::None;
	if (bMove && TargetPlace == nullptr)
	{
		const FVector From = (AI->GetPawn() != nullptr) ? AI->GetPawn()->GetActorLocation() : FVector::ZeroVector;

		// 이 행동이 쓸 점 종류 전부(이동 스텝들의 합집합) → 그걸 다 가진 건물만 후보.
		TSet<EPlacePoint> Required;
		for (const FActionStep& S : Sequence)
		{
			if (S.Point != EPlacePoint::None)
			{
				Required.Add(S.Point);
			}
		}

		TargetPlace = FindNearestPlace(GetWorld(), From, Step.Category, Required);
		if (TargetPlace == nullptr)
		{
			Stop(); // 조건 맞는 건물 없음 → 종료.
			return;
		}
	}

	AI->RunBehaviorTree(Step.BT);

	// 이동 스텝이면 이 스텝의 지점 위치를 MoveGoal에 쓴다(스톡 MoveTo가 읽음).
	if (bMove && TargetPlace != nullptr)
	{
		const TArray<FVector> Locs = TargetPlace->GetPoints(Step.Point);
		if (Locs.Num() > 0)
		{
			// 같은 종류 지점이 여럿이면 NPC에 최근접한 것을 목적지로. (점유는 나중)
			const FVector From = (AI->GetPawn() != nullptr) ? AI->GetPawn()->GetActorLocation() : FVector::ZeroVector;
			FVector Nearest = Locs[0];
			double BestDistSq = FVector::DistSquared(From, Nearest);
			for (int32 i = 1; i < Locs.Num(); ++i)
			{
				const double DistSq = FVector::DistSquared(From, Locs[i]);
				if (DistSq < BestDistSq)
				{
					BestDistSq = DistSq;
					Nearest = Locs[i];
				}
			}

			if (UBlackboardComponent* BB = AI->GetBlackboardComponent())
			{
				BB->SetValueAsVector(MoveGoalKey, Nearest);
			}
		}
	}
}

void UActionRunnerComponent::Stop()
{
	Sequence.Empty();
	Index = 0;
	TargetPlace = nullptr;

	if (AAIController* AI = Cast<AAIController>(GetOwner()))
	{
		if (UBrainComponent* Brain = AI->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("ActionRunner: sequence done"));
		}
	}
}

UPlaceComponent* UActionRunnerComponent::FindNearestPlace(const UWorld* World, const FVector& From,
	EBuildingType Category, const TSet<EPlacePoint>& RequiredPoints)
{
	if (World == nullptr)
	{
		return nullptr;
	}

	UPlaceComponent* Best = nullptr;
	double BestDistSq = TNumericLimits<double>::Max();

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		UPlaceComponent* Place = (Actor != nullptr) ? Actor->FindComponentByClass<UPlaceComponent>() : nullptr;
		if (Place == nullptr || Place->Type != Category)
		{
			continue;
		}

		// RequiredPoints의 모든 종류를 이 건물이 가져야 통과(입구만 있고 좌석 없는 건물 배제).
		bool bHasAll = true;
		for (const EPlacePoint Kind : RequiredPoints)
		{
			if (Place->GetPoints(Kind).Num() == 0)
			{
				bHasAll = false;
				break;
			}
		}
		if (!bHasAll)
		{
			continue;
		}

		const double DistSq = FVector::DistSquared(From, Actor->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Place;
		}
	}

	return Best;
}
