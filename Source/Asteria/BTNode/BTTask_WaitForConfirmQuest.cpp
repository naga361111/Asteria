// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_WaitForConfirmQuest.h"

#include "AIController.h"
#include "NPC/AsteriaNpc.h"
#include "GameState/AsteriaGameState.h"
#include "GameState/Components/QuestService.h"
#include "GameState/Components/CounterService.h"

// 이 태스크의 실행별 상태. 노드는 트리를 쓰는 모든 AI가 공유하는 단일 인스턴스라
// 멤버 변수에 두면 서로 덮어쓴다 → NodeMemory에 담는다.
struct FBTWaitForConfirmQuestMemory
{
	TWeakObjectPtr<UQuestService> QuestService;
	FDelegateHandle AcceptedHandle;
	int32 WaitingAssignmentId = INDEX_NONE;
};

UBTTask_WaitForConfirmQuest::UBTTask_WaitForConfirmQuest()
{
	NodeName = TEXT("Wait For Confirm Quest");

	// bNotifyTaskFinished는 기본 false다 — 안 켜면 OnTaskFinished가 호출되지 않아
	// 아래에서 거는 구독이 영영 해제되지 않는다(실행할 때마다 람다가 쌓인다).
	// 이 매크로가 오버라이드 여부를 보고 Tick/TaskFinished 플래그를 맞춰준다.
	INIT_TASK_NODE_NOTIFY_FLAGS();
}

EBTNodeResult::Type UBTTask_WaitForConfirmQuest::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	UCounterService* Counter = GS->CounterService;
	if (Counter == nullptr) return EBTNodeResult::Failed;

	// 내가 기다릴 Assignment = 파티에 내 NpcId가 들고 아직 제출 전(Assigned)인 Assignment.
	// 상태를 안 보면 이미 올린 것·수주 확정된 것까지 집어 엉뚱한 Assignment를 기다린다.
	// TODO: SelectQuest가 AssignmentId를 넘겨주는 경계가 서면 그 값을 받아 이 순회를 대체.
	int32 WaitingAssignmentId = INDEX_NONE;
	for (const FQuestAssignment& Assignment : Service->QuestAssignments)
	{
		if (Assignment.State == EQuestAssignmentState::Assigned && Assignment.Party.Contains(Npc->NpcId))
		{
			WaitingAssignmentId = Assignment.AssignmentId;
			break;
		}
	}
	if (WaitingAssignmentId == INDEX_NONE) return EBTNodeResult::Failed; // 집은 게 없으면 기다릴 대상도 없음

	FBTWaitForConfirmQuestMemory* Mem = CastInstanceNodeMemory<FBTWaitForConfirmQuestMemory>(NodeMemory);
	Mem->QuestService = Service;
	Mem->WaitingAssignmentId = WaitingAssignmentId;

	// 밖에서 깨우고(QuestService가 Submitted→Accepted 시 OnQuestAssignmentAccepted.Broadcast) → 안에서 끝낸다(이 람다가 FinishLatentTask 호출).
	// 브로드캐스트는 어느 Assignment든 오므로 내 AssignmentId만 필터.
	Mem->AcceptedHandle = Service->OnQuestAssignmentAccepted.AddLambda(
		[this, &OwnerComp, Mem](int32 AcceptedAssignmentId)
		{
			if (AcceptedAssignmentId == Mem->WaitingAssignmentId)
			{
				UE_LOG(LogTemp, Warning, TEXT("Assignment Accepted: %d"), AcceptedAssignmentId)
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			}
		});

	// 구독을 먼저, 제출은 그 다음 — 순서가 바뀌면 제출 직후의 컨펌을 놓친다.
	// 실패하면 기다릴 이유가 없다 → Failed. (구독 해제는 OnTaskFinished가 책임)
	if (!Counter->SubmitQuestAssignment(WaitingAssignmentId))
	{
		UE_LOG(LogTemp, Warning, TEXT("SubmitQuestAssignment failed. Assignment:%d"), WaitingAssignmentId)
		return EBTNodeResult::Failed;
	}

	UE_LOG(LogTemp, Warning, TEXT("Wait For Assignment:%d Confirm"), WaitingAssignmentId)
	return EBTNodeResult::InProgress;
}

void UBTTask_WaitForConfirmQuest::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	// 성공/실패/Abort 어느 경로로 끝나든 여기로 온다 → 구독을 반드시 해제(댕글링 방지).
	FBTWaitForConfirmQuestMemory* Mem = CastInstanceNodeMemory<FBTWaitForConfirmQuestMemory>(NodeMemory);
	if (Mem->QuestService.IsValid() && Mem->AcceptedHandle.IsValid())
	{
		Mem->QuestService->OnQuestAssignmentAccepted.Remove(Mem->AcceptedHandle);
	}
	Mem->AcceptedHandle.Reset();
	Mem->QuestService.Reset();
	Mem->WaitingAssignmentId = INDEX_NONE;

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

uint16 UBTTask_WaitForConfirmQuest::GetInstanceMemorySize() const
{
	return sizeof(FBTWaitForConfirmQuestMemory);
}

void UBTTask_WaitForConfirmQuest::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
	// NodeMemory는 raw. 비POD 멤버(TWeakObjectPtr/FDelegateHandle)를 위해 placement new로 초기화.
	// (조기 Fail 리턴 경로에서도 OnTaskFinished가 유효한 핸들 상태를 보게 하는 안전장치)
	new (NodeMemory) FBTWaitForConfirmQuestMemory();
}

void UBTTask_WaitForConfirmQuest::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const
{
	CastInstanceNodeMemory<FBTWaitForConfirmQuestMemory>(NodeMemory)->~FBTWaitForConfirmQuestMemory();
}
