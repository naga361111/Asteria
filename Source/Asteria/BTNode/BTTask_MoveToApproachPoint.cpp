// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_MoveToApproachPoint.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
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

UBTTask_MoveToApproachPoint::UBTTask_MoveToApproachPoint()
{
	NodeName = TEXT("Move To Approach Point");

	// 실행 중 상태를 멤버에 저장하므로 트리 컴포넌트마다 별도 인스턴스가 필요하다.
	bCreateNodeInstance = true;

	// 회전 페이즈에서만 틱을 사용한다.
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_MoveToApproachPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	if (Pawn == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	// (1) 접근 지점 탐색.
	TargetPoint = FindNearestApproachPoint(Pawn->GetWorld(), TargetType, Pawn->GetActorLocation());
	if (TargetPoint == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	CachedOwnerComp = &OwnerComp;
	bFacing = false;

	// (2) 이동 요청.
	const EPathFollowingRequestResult::Type MoveResult = Controller->MoveToActor(TargetPoint, AcceptableRadius);
	switch (MoveResult)
	{
	case EPathFollowingRequestResult::Failed:
		return EBTNodeResult::Failed;

	case EPathFollowingRequestResult::AlreadyAtGoal:
		// 이미 도착 → 바로 회전 페이즈로.
		bFacing = true;
		if (InterpSpeed <= 0.0f)
		{
			Pawn->SetActorRotation(FRotator(0.0f, TargetPoint->GetActorRotation().Yaw, 0.0f));
			return EBTNodeResult::Succeeded;
		}
		return EBTNodeResult::InProgress;

	case EPathFollowingRequestResult::RequestSuccessful:
	default:
		MoveRequestID = Controller->GetCurrentMoveRequestID();
		Controller->ReceiveMoveCompleted.AddDynamic(this, &UBTTask_MoveToApproachPoint::OnMoveCompleted);
		return EBTNodeResult::InProgress;
	}
}

void UBTTask_MoveToApproachPoint::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	// 이전 실행의 스테일 콜백 무시.
	if (!RequestID.IsEquivalent(MoveRequestID))
	{
		return;
	}

	UBehaviorTreeComponent* OwnerComp = CachedOwnerComp.Get();
	if (OwnerComp == nullptr)
	{
		return;
	}

	UnbindMoveCompleted();

	APawn* Pawn = OwnerComp->GetAIOwner() ? OwnerComp->GetAIOwner()->GetPawn() : nullptr;
	if (Pawn == nullptr || TargetPoint == nullptr)
	{
		FinishLatentTask(*OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 도착 실패(막힘·중단 등)면 태스크도 실패.
	if (Result != EPathFollowingResult::Success)
	{
		FinishLatentTask(*OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// (3) 회전 페이즈 진입.
	bFacing = true;
	if (InterpSpeed <= 0.0f)
	{
		Pawn->SetActorRotation(FRotator(0.0f, TargetPoint->GetActorRotation().Yaw, 0.0f));
		FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
	}
	// 보간 모드는 TickTask가 마무리한다.
}

void UBTTask_MoveToApproachPoint::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (!bFacing)
	{
		return;
	}

	APawn* Pawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (Pawn == nullptr || TargetPoint == nullptr)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (StepFacing(*Pawn, DeltaSeconds))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UBTTask_MoveToApproachPoint::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 브랜치 중단 시 진행 중인 이동을 반드시 멈추고 콜백을 뗀다.
	if (AAIController* Controller = OwnerComp.GetAIOwner())
	{
		if (MoveRequestID.IsValid())
		{
			Controller->StopMovement();
		}
	}
	UnbindMoveCompleted();
	bFacing = false;
	return EBTNodeResult::Aborted;
}

void UBTTask_MoveToApproachPoint::UnbindMoveCompleted()
{
	if (UBehaviorTreeComponent* OwnerComp = CachedOwnerComp.Get())
	{
		if (AAIController* Controller = OwnerComp->GetAIOwner())
		{
			Controller->ReceiveMoveCompleted.RemoveDynamic(this, &UBTTask_MoveToApproachPoint::OnMoveCompleted);
		}
	}
	MoveRequestID = FAIRequestID::InvalidRequest;
}

bool UBTTask_MoveToApproachPoint::StepFacing(APawn& Pawn, float DeltaSeconds) const
{
	const float DesiredYaw = TargetPoint->GetActorRotation().Yaw;
	const FRotator Current = Pawn.GetActorRotation();
	const FRotator NewRot = FMath::RInterpTo(Current, FRotator(0.0f, DesiredYaw, 0.0f), DeltaSeconds, InterpSpeed);
	Pawn.SetActorRotation(NewRot);

	if (FMath::Abs(FMath::FindDeltaAngleDegrees(NewRot.Yaw, DesiredYaw)) <= AngleTolerance)
	{
		Pawn.SetActorRotation(FRotator(0.0f, DesiredYaw, 0.0f));
		return true;
	}
	return false;
}
