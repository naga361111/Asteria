// Fill out your copyright notice in the Description page of Project Settings.


#include "BTNode/BTTask_FindApproachPoint.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	// NPC 기준 해당 종류의 가장 가까운 접근 지점(2D 거리). 없으면 null.
	AApproachPointActor* FindNearestApproachPoint(UWorld* World, EApproachPointType Type, const FVector& Origin)
	{
		TArray<AActor*> Points;
		UGameplayStatics::GetAllActorsOfClass(World, AApproachPointActor::StaticClass(), Points);

		AApproachPointActor* Best = nullptr;
		float BestDistSq = TNumericLimits<float>::Max();
		for (AActor* Actor : Points)
		{
			AApproachPointActor* Point = Cast<AApproachPointActor>(Actor);
			if (Point == nullptr || Point->PointType != Type)
			{
				continue;
			}

			const float DistSq = FVector::DistSquared2D(Origin, Point->GetActorLocation());
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				Best = Point;
			}
		}
		return Best;
	}
}

UBTTask_FindApproachPoint::UBTTask_FindApproachPoint()
{
	NodeName = TEXT("Find Approach Point");

	// ResultKey를 Object 타입 키만 선택 가능하도록 필터.
	ResultKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindApproachPoint, ResultKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_FindApproachPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* Pawn = AIController != nullptr ? AIController->GetPawn() : nullptr;
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (Pawn == nullptr || Blackboard == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	AApproachPointActor* Found = FindNearestApproachPoint(Pawn->GetWorld(), TargetType, Pawn->GetActorLocation());
	if (Found == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	Blackboard->SetValueAsObject(ResultKey.SelectedKeyName, Found);
	return EBTNodeResult::Succeeded;
}
