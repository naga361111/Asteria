// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_SettleQuest.h"

#include "AIController.h"
#include "NPC/AsteriaNpc.h"
#include "GameState/AsteriaGameState.h"
#include "GameState/Components/QuestService.h"

// 이 태스크의 실행별 상태. 노드는 트리를 쓰는 모든 AI가 공유하는 단일 인스턴스라
// 멤버 변수에 두면 서로 덮어쓴다 → NodeMemory에 담는다.
struct FBTSettleQuestMemory
{
	TWeakObjectPtr<UQuestService> QuestService;
	// 이 실행이 대기열에 올린 Assignment. 올리기 전·실패 시엔 INDEX_NONE으로 남아
	// OnTaskFinished가 남의 원소를 내리지 않는다.
	int32 WaitingAssignmentId = INDEX_NONE;
};

UBTTask_SettleQuest::UBTTask_SettleQuest()
{
	NodeName = TEXT("Settle Quest");

	// bNotifyTaskFinished는 기본 false다 — 안 켜면 OnTaskFinished가 호출되지 않아
	// Abort로 끝난 실행이 대기열에 사본을 남긴 채 사라진다.
	INIT_TASK_NODE_NOTIFY_FLAGS();
}

EBTNodeResult::Type UBTTask_SettleQuest::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// BT는 서버에서만 도는 전제. 월드/NpcId는 OwnerComp의 AI 오너·폰에서 얻는다.
	AAIController* AICon = OwnerComp.GetAIOwner();
	AAsteriaNpc* Npc = AICon ? Cast<AAsteriaNpc>(AICon->GetPawn()) : nullptr;
	if (Npc == nullptr) return EBTNodeResult::Failed;

	AAsteriaGameState* GS = Npc->GetWorld()->GetGameState<AAsteriaGameState>();
	UQuestService* Service = GS ? GS->QuestService : nullptr;
	if (Service == nullptr) return EBTNodeResult::Failed;

	// 무엇을 정산받을지는 소유자에게 묻는다. 완료된 것만 — 그 앞 단계는 아직 정산 대상이 아니다.
	const FQuestAssignment* Assignment = Service->FindQuestAssignmentByNpc(Npc->NpcId, EQuestAssignmentState::Cleared);
	if (Assignment == nullptr) return EBTNodeResult::Failed;

	// 포인터가 아니라 id만 들고 간다 — QuestAssignments가 바뀌면 위 포인터는 그 즉시 무효.
	const int32 AssignmentId = Assignment->AssignmentId;

	FBTSettleQuestMemory* Mem = CastInstanceNodeMemory<FBTSettleQuestMemory>(NodeMemory);
	Mem->QuestService = Service;

	// 등재의 검증·기록은 소유자(QuestService)가 서버 권위로 한다 — 배열을 여기서 직접 건드리지 않는다.
	if (!Service->EnqueueSettleQuestAssignment(AssignmentId))
	{
		UE_LOG(LogTemp, Warning, TEXT("EnqueueSettleQuestAssignment failed. Assignment:%d"), AssignmentId)
		return EBTNodeResult::Failed;
	}

	// 등재에 성공한 뒤에야 기록한다 — 회수 책임은 실제로 올린 실행에만 있다.
	Mem->WaitingAssignmentId = AssignmentId;

	// 밖에서 깨우기 전까지 끝나지 않는다. 깨우는 쪽(창구 정산 확정)은 아직 없다.
	UE_LOG(LogTemp, Warning, TEXT("Wait For Assignment:%d Settle"), AssignmentId)
	return EBTNodeResult::InProgress;
}

void UBTTask_SettleQuest::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	// 성공/실패/Abort 어느 경로로 끝나든 여기로 온다 → 올린 사본을 반드시 회수(대기열 누수 방지).
	FBTSettleQuestMemory* Mem = CastInstanceNodeMemory<FBTSettleQuestMemory>(NodeMemory);
	if (Mem->QuestService.IsValid() && Mem->WaitingAssignmentId != INDEX_NONE)
	{
		Mem->QuestService->DequeueSettleQuestAssignment(Mem->WaitingAssignmentId);
	}
	Mem->WaitingAssignmentId = INDEX_NONE;
	Mem->QuestService.Reset();

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

uint16 UBTTask_SettleQuest::GetInstanceMemorySize() const
{
	return sizeof(FBTSettleQuestMemory);
}

void UBTTask_SettleQuest::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
	// NodeMemory는 raw. 비POD 멤버(TWeakObjectPtr)를 위해 placement new로 초기화.
	// (조기 Fail 리턴 경로에서도 OnTaskFinished가 유효한 상태를 보게 하는 안전장치)
	new (NodeMemory) FBTSettleQuestMemory();
}

void UBTTask_SettleQuest::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const
{
	CastInstanceNodeMemory<FBTSettleQuestMemory>(NodeMemory)->~FBTSettleQuestMemory();
}
