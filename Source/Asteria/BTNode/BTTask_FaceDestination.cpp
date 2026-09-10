// Fill out your copyright notice in the Description page of Project Settings.


#include "BTNode/BTTask_FaceDestination.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"

UBTTask_FaceDestination::UBTTask_FaceDestination()
{
	NodeName = TEXT("Face Destination");
	bNotifyTick = true;

	// Object(액터) 키만 선택 가능하도록 필터링한다.
	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FaceDestination, TargetKey), AActor::StaticClass());
}

void UBTTask_FaceDestination::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		TargetKey.ResolveSelectedKey(*BBAsset);
	}
}

EBTNodeResult::Type UBTTask_FaceDestination::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (Controller == nullptr || Pawn == nullptr || BB == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	AActor* Dest = Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName));
	if (Dest == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	FBTFaceDestinationMemory* Memory = reinterpret_cast<FBTFaceDestinationMemory*>(NodeMemory);
	Memory->DesiredYaw = Dest->GetActorRotation().Yaw;

	// 즉시 스냅 모드.
	if (InterpSpeed <= 0.0f)
	{
		ApplyYaw(Controller, Pawn, Memory->DesiredYaw);
		return EBTNodeResult::Succeeded;
	}

	// 보간 모드: TickTask에서 목표각까지 회전시킨다.
	return EBTNodeResult::InProgress;
}

void UBTTask_FaceDestination::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	if (Controller == nullptr || Pawn == nullptr)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	FBTFaceDestinationMemory* Memory = reinterpret_cast<FBTFaceDestinationMemory*>(NodeMemory);

	const FRotator Current = Pawn->GetActorRotation();
	const FRotator Target(0.0f, Memory->DesiredYaw, 0.0f);
	const FRotator NewRot = FMath::RInterpTo(Current, Target, DeltaSeconds, InterpSpeed);

	ApplyYaw(Controller, Pawn, NewRot.Yaw);

	// 목표각 허용 범위 안이면 스냅으로 마무리하고 성공.
	if (FMath::Abs(FMath::FindDeltaAngleDegrees(NewRot.Yaw, Memory->DesiredYaw)) <= AngleTolerance)
	{
		ApplyYaw(Controller, Pawn, Memory->DesiredYaw);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

void UBTTask_FaceDestination::ApplyYaw(AAIController* Controller, APawn* Pawn, float Yaw)
{
	const FRotator YawRot(0.0f, Yaw, 0.0f);

	// bOrientRotationToMovement 모드: 폰 액터 회전이 권위(정지 상태라 유지됨).
	Pawn->SetActorRotation(YawRot);
	// bUseControllerRotationYaw 모드: 컨트롤 회전이 매 틱 폰을 덮으므로 함께 맞춘다.
	Controller->SetControlRotation(YawRot);
}
