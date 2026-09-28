// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_WaitForSettleConfirm.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "NPC/AsteriaNpc.h"
#include "GameState/AsteriaGameState.h"
#include "GameState/Components/QuestService.h"

UBTTask_WaitForSettleConfirm::UBTTask_WaitForSettleConfirm()
{
	NodeName = TEXT("Wait For Settle Confirm");

	// 실행 중 상태(이동·회전·구독)를 멤버에 저장하므로 트리 컴포넌트마다 별도 인스턴스가 필요하다.
	bCreateNodeInstance = true;

	// 회전 페이즈에서만 틱을 사용한다.
	bNotifyTick = true;

	// 성공/실패/Abort 공통 정리를 OnTaskFinished에서 한다.
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_WaitForSettleConfirm::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// BT는 서버에서만 도는 전제. 월드/NpcId는 OwnerComp의 AI 오너·폰에서 얻는다.
	// (노드의 GetWorld()는 Outer가 월드가 아니라 신뢰 불가)
	AAIController* AICon = OwnerComp.GetAIOwner();
	AAsteriaNpc* Npc = AICon ? Cast<AAsteriaNpc>(AICon->GetPawn()) : nullptr;
	if (Npc == nullptr) return EBTNodeResult::Failed;

	AAsteriaGameState* GS = Npc->GetWorld()->GetGameState<AAsteriaGameState>();
	if (GS == nullptr) return EBTNodeResult::Failed;

	UQuestService* Service = GS->QuestService;
	if (Service == nullptr) return EBTNodeResult::Failed;

	// 내가 기다릴 Assignment = 내 NpcId가 파티에 있고 보상 대기 제출함에 올라간(SubmitForSettled) 것.
	const FQuestAssignment* Assignment = Service->FindQuestAssignmentByNpc(Npc->NpcId, EQuestAssignmentState::SubmitForSettled);
	if (Assignment == nullptr)
	{
		// 제출과 대기 사이에 이미 컨펌됐으면 기다릴 것 없이 성공(이동 없음). 둘 다 없으면 기다릴 대상이 없다.
		return Service->FindQuestAssignmentByNpc(Npc->NpcId, EQuestAssignmentState::SettleConfirmed)
			? EBTNodeResult::Succeeded
			: EBTNodeResult::Failed;
	}

	QuestService = Service;
	// 포인터가 아니라 id만 들고 간다 — QuestAssignments가 바뀌면 위 포인터는 그 즉시 무효.
	WaitingAssignmentId = Assignment->AssignmentId;
	CachedOwnerComp = &OwnerComp;
	bFacing = false;

	// 밖에서 깨우고(QuestService가 SubmitForSettled→SettleConfirmed 시 OnQuestAssignmentSettled.Broadcast) → 안에서 끝낸다(이 람다가 FinishLatentTask 호출).
	// 브로드캐스트는 어느 Assignment든 오므로 내 AssignmentId만 필터.
	// 이동 중이든 도착 후든 즉시 끝낸다 — 걸어가던 이동은 OnTaskFinished가 멈춘다.
	SettledHandle = Service->OnQuestAssignmentSettled.AddLambda(
		[this, &OwnerComp](int32 SettledAssignmentId)
		{
			if (SettledAssignmentId == WaitingAssignmentId)
			{
				UE_LOG(LogTemp, Warning, TEXT("Assignment Settle Confirmed: %d"), SettledAssignmentId)
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			}
		});

	UE_LOG(LogTemp, Warning, TEXT("Wait For Assignment:%d Settle"), WaitingAssignmentId)

	// 예약되지 않은 대기 지점 탐색. 대기 자리는 연출이라 지점이 없어도 실패하지 않고 제자리에서 기다린다.
	TargetPoint = AApproachPointActor::FindNearestFree(Npc->GetWorld(), TargetType, Npc->GetActorLocation());
	if (TargetPoint == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No wait point found. Waiting in place. Assignment:%d"), WaitingAssignmentId)
		return EBTNodeResult::InProgress;
	}

	// 고르는 즉시 예약 — 다음 Npc는 이 자리를 건너뛰고 다음으로 가까운 빈 자리를 고른다. 해제는 OnTaskFinished.
	TargetPoint->Occupant = Npc;

	// 대기 지점으로 이동 요청.
	const EPathFollowingRequestResult::Type MoveResult = AICon->MoveToActor(TargetPoint, AcceptableRadius);
	switch (MoveResult)
	{
	case EPathFollowingRequestResult::Failed:
		UE_LOG(LogTemp, Warning, TEXT("Move to wait point failed. Waiting in place. Assignment:%d"), WaitingAssignmentId)
		break;

	case EPathFollowingRequestResult::AlreadyAtGoal:
		// 이미 도착 → 바로 회전 페이즈로.
		if (InterpSpeed <= 0.0f)
		{
			Npc->SetActorRotation(FRotator(0.0f, TargetPoint->GetActorRotation().Yaw, 0.0f));
		}
		else
		{
			bFacing = true;
		}
		break;

	case EPathFollowingRequestResult::RequestSuccessful:
	default:
		MoveRequestID = AICon->GetCurrentMoveRequestID();
		AICon->ReceiveMoveCompleted.AddDynamic(this, &UBTTask_WaitForSettleConfirm::OnMoveCompleted);
		break;
	}
	return EBTNodeResult::InProgress;
}

void UBTTask_WaitForSettleConfirm::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
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

	// 도착 실패(막힘·중단 등)여도 태스크를 끝내지 않고 제자리에서 컨펌을 기다린다.
	if (Result != EPathFollowingResult::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("Move to wait point failed. Waiting in place. Assignment:%d"), WaitingAssignmentId)
		return;
	}

	APawn* Pawn = OwnerComp->GetAIOwner() ? OwnerComp->GetAIOwner()->GetPawn() : nullptr;
	if (Pawn == nullptr || TargetPoint == nullptr)
	{
		return;
	}

	// 회전 페이즈 진입. 보간 모드는 TickTask가 마무리한다.
	if (InterpSpeed <= 0.0f)
	{
		Pawn->SetActorRotation(FRotator(0.0f, TargetPoint->GetActorRotation().Yaw, 0.0f));
	}
	else
	{
		bFacing = true;
	}
}

void UBTTask_WaitForSettleConfirm::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (!bFacing)
	{
		return;
	}

	APawn* Pawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (Pawn == nullptr || TargetPoint == nullptr)
	{
		bFacing = false;
		return;
	}

	// 다 돌면 회전만 끝내고 컨펌 대기는 계속한다.
	if (StepFacing(*Pawn, DeltaSeconds))
	{
		bFacing = false;
	}
}

void UBTTask_WaitForSettleConfirm::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	// 성공/실패/Abort 어느 경로로 끝나든 여기로 온다 → 구독을 반드시 해제(댕글링 방지).
	if (QuestService.IsValid() && SettledHandle.IsValid())
	{
		QuestService->OnQuestAssignmentSettled.Remove(SettledHandle);
	}
	SettledHandle.Reset();
	QuestService.Reset();
	WaitingAssignmentId = INDEX_NONE;

	// 이 태스크의 이동이 진행 중이면 멈추고 콜백을 뗀다.
	if (MoveRequestID.IsValid())
	{
		if (AAIController* Controller = OwnerComp.GetAIOwner())
		{
			Controller->StopMovement();
		}
	}
	UnbindMoveCompleted();
	// 이 태스크가 예약한 자리를 해제한다.
	if (TargetPoint != nullptr)
	{
		TargetPoint->Occupant.Reset();
	}
	TargetPoint = nullptr;
	CachedOwnerComp.Reset();
	bFacing = false;

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBTTask_WaitForSettleConfirm::UnbindMoveCompleted()
{
	if (UBehaviorTreeComponent* OwnerComp = CachedOwnerComp.Get())
	{
		if (AAIController* Controller = OwnerComp->GetAIOwner())
		{
			Controller->ReceiveMoveCompleted.RemoveDynamic(this, &UBTTask_WaitForSettleConfirm::OnMoveCompleted);
		}
	}
	MoveRequestID = FAIRequestID::InvalidRequest;
}

bool UBTTask_WaitForSettleConfirm::StepFacing(APawn& Pawn, float DeltaSeconds) const
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
