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

	UBehaviorTree* BT = Sequence[Index].BT;
	if (BT == nullptr)
	{
		Advance(); // null 원자 건너뜀.
		return;
	}

	AI->RunBehaviorTree(BT);
	if (UBlackboardComponent* BB = AI->GetBlackboardComponent())
	{
		BB->SetValueAsVector(MoveGoalKey, Target);
	}
}

void UActionRunnerComponent::Stop()
{
	Sequence.Empty();
	Index = 0;

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
