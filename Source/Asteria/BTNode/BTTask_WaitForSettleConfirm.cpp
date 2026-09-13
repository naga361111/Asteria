// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_WaitForSettleConfirm.h"

#include "AIController.h"
#include "NPC/AsteriaNpc.h"
#include "GameState/AsteriaGameState.h"
#include "GameState/Components/QuestService.h"
#include "GameState/Components/CounterService.h"

// 이 태스크의 실행별 상태. 노드는 트리를 쓰는 모든 AI가 공유하는 단일 인스턴스라
// 멤버 변수에 두면 서로 덮어쓴다 → NodeMemory에 담는다.
struct FBTWaitForSettleConfirmMemory
{
	TWeakObjectPtr<UQuestService> QuestService;
	FDelegateHandle SettledHandle;
	int32 WaitingAssignmentId = INDEX_NONE;
};

UBTTask_WaitForSettleConfirm::UBTTask_WaitForSettleConfirm()
{
	NodeName = TEXT("Wait For Settle Confirm");

	// bNotifyTaskFinished는 기본 false다 — 안 켜면 OnTaskFinished가 호출되지 않아
	// 아래에서 거는 구독이 영영 해제되지 않는다(실행할 때마다 람다가 쌓인다).
	INIT_TASK_NODE_NOTIFY_FLAGS();
}

EBTNodeResult::Type UBTTask_WaitForSettleConfirm::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// BT는 서버에서만 도는 전제. 월드/NpcId는 OwnerComp의 AI 오너·폰에서 얻는다.
	AAIController* AICon = OwnerComp.GetAIOwner();
	AAsteriaNpc* Npc = AICon ? Cast<AAsteriaNpc>(AICon->GetPawn()) : nullptr;
	if (Npc == nullptr) return EBTNodeResult::Failed;

	AAsteriaGameState* GS = Npc->GetWorld()->GetGameState<AAsteriaGameState>();
	if (GS == nullptr) return EBTNodeResult::Failed;

	UQuestService* Service = GS->QuestService;
	if (Service == nullptr) return EBTNodeResult::Failed;

	UCounterService* Counter = GS->CounterService;
	if (Counter == nullptr) return EBTNodeResult::Failed;

	// 내가 올릴 Assignment = 내 NpcId가 파티에 있고 수행을 마친(Cleared) 것.
	// 상태로 좁히지 않으면 아직 수행 중인 것·이미 올린 것까지 집는다.
	const FQuestAssignment* Assignment = Service->FindQuestAssignmentByNpc(Npc->NpcId, EQuestAssignmentState::Cleared);
	if (Assignment == nullptr) return EBTNodeResult::Failed;

	FBTWaitForSettleConfirmMemory* Mem = CastInstanceNodeMemory<FBTWaitForSettleConfirmMemory>(NodeMemory);
	Mem->QuestService = Service;
	// 포인터가 아니라 id만 들고 간다 — QuestAssignments가 바뀌면 위 포인터는 그 즉시 무효.
	Mem->WaitingAssignmentId = Assignment->AssignmentId;

	// 밖에서 깨우고(QuestService가 SubmitForSettled→Settled 시 OnQuestAssignmentSettled.Broadcast) → 안에서 끝낸다(이 람다가 FinishLatentTask 호출).
	// 브로드캐스트는 어느 Assignment든 오므로 내 AssignmentId만 필터.
	Mem->SettledHandle = Service->OnQuestAssignmentSettled.AddLambda(
		[this, &OwnerComp, Mem](int32 SettledAssignmentId)
		{
			if (SettledAssignmentId == Mem->WaitingAssignmentId)
			{
				UE_LOG(LogTemp, Warning, TEXT("Assignment Settled: %d"), SettledAssignmentId)
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			}
		});

	// 구독을 먼저, 제출은 그 다음 — 순서가 바뀌면 제출 직후의 정산 확정을 놓친다.
	// 실패하면 기다릴 이유가 없다 → Failed. (구독 해제는 OnTaskFinished가 책임)
	if (!Counter->SubmitForSettleQuestAssignment(Mem->WaitingAssignmentId))
	{
		UE_LOG(LogTemp, Warning, TEXT("SubmitForSettleQuestAssignment failed. Assignment:%d"), Mem->WaitingAssignmentId)
		return EBTNodeResult::Failed;
	}

	UE_LOG(LogTemp, Warning, TEXT("Wait For Assignment:%d Settle"), Mem->WaitingAssignmentId)
	return EBTNodeResult::InProgress;
}

void UBTTask_WaitForSettleConfirm::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	// 성공/실패/Abort 어느 경로로 끝나든 여기로 온다 → 구독을 반드시 해제(댕글링 방지).
	FBTWaitForSettleConfirmMemory* Mem = CastInstanceNodeMemory<FBTWaitForSettleConfirmMemory>(NodeMemory);
	if (Mem->QuestService.IsValid() && Mem->SettledHandle.IsValid())
	{
		Mem->QuestService->OnQuestAssignmentSettled.Remove(Mem->SettledHandle);
	}
	Mem->SettledHandle.Reset();
	Mem->QuestService.Reset();
	Mem->WaitingAssignmentId = INDEX_NONE;

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

uint16 UBTTask_WaitForSettleConfirm::GetInstanceMemorySize() const
{
	return sizeof(FBTWaitForSettleConfirmMemory);
}

void UBTTask_WaitForSettleConfirm::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
	// NodeMemory는 raw. 비POD 멤버(TWeakObjectPtr/FDelegateHandle)를 위해 placement new로 초기화.
	// (조기 Fail 리턴 경로에서도 OnTaskFinished가 유효한 핸들 상태를 보게 하는 안전장치)
	new (NodeMemory) FBTWaitForSettleConfirmMemory();
}

void UBTTask_WaitForSettleConfirm::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const
{
	CastInstanceNodeMemory<FBTWaitForSettleConfirmMemory>(NodeMemory)->~FBTWaitForSettleConfirmMemory();
}
