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
	if (Pawn == nullptr || BB == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	AActor* Dest = Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName));
	if (Dest == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	// 즉시 스냅 모드: 목적지 yaw로 바로 맞추고 성공.
	if (InterpSpeed <= 0.0f)
	{
		Pawn->SetActorRotation(FRotator(0.0f, Dest->GetActorRotation().Yaw, 0.0f));
		return EBTNodeResult::Succeeded;
	}

	// 보간 모드: TickTask에서 목표각까지 회전시킨다. 목적지는 정적이므로 매 틱 다시 읽는다.
	return EBTNodeResult::InProgress;
}

void UBTTask_FaceDestination::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AActor* Dest = BB ? Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName)) : nullptr;
	if (Pawn == nullptr || Dest == nullptr)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	const float DesiredYaw = Dest->GetActorRotation().Yaw;
	const FRotator Current = Pawn->GetActorRotation();
	const FRotator NewRot = FMath::RInterpTo(Current, FRotator(0.0f, DesiredYaw, 0.0f), DeltaSeconds, InterpSpeed);
	Pawn->SetActorRotation(NewRot);

	// 목표각 허용 범위 안이면 스냅으로 마무리하고 성공.
	if (FMath::Abs(FMath::FindDeltaAngleDegrees(NewRot.Yaw, DesiredYaw)) <= AngleTolerance)
	{
		Pawn->SetActorRotation(FRotator(0.0f, DesiredYaw, 0.0f));
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
