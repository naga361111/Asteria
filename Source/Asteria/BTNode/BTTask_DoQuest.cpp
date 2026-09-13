// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_DoQuest.h"

#include "AIController.h"
#include "NPC/AsteriaNpc.h"
#include "GameState/AsteriaGameState.h"
#include "GameState/Components/QuestService.h"

// 이 태스크의 실행별 상태. 노드는 트리를 쓰는 모든 AI가 공유하는 단일 인스턴스라
// 멤버 변수에 두면 서로 덮어쓴다 → NodeMemory에 담는다.
struct FBTDoQuestMemory
{
	TWeakObjectPtr<UQuestService> QuestService;
	int32 AssignmentId = INDEX_NONE;
	float RemainingTime = 0.f;
};

UBTTask_DoQuest::UBTTask_DoQuest()
{
	NodeName = TEXT("Do Quest");

	// 오버라이드한 통지(TickTask)에 맞춰 bNotifyTick 등을 맞춰준다.
	INIT_TASK_NODE_NOTIFY_FLAGS();
}

EBTNodeResult::Type UBTTask_DoQuest::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// BT는 서버에서만 도는 전제. 월드/NpcId는 OwnerComp의 AI 오너·폰에서 얻는다.
	AAIController* AICon = OwnerComp.GetAIOwner();
	AAsteriaNpc* Npc = AICon ? Cast<AAsteriaNpc>(AICon->GetPawn()) : nullptr;
	if (Npc == nullptr) return EBTNodeResult::Failed;

	AAsteriaGameState* GS = Npc->GetWorld()->GetGameState<AAsteriaGameState>();
	UQuestService* Service = GS ? GS->QuestService : nullptr;
	if (Service == nullptr) return EBTNodeResult::Failed;

	// 무엇을 수행할지는 소유자에게 묻는다. 수주 확정된 것만 — Assigned/Submitted는 아직 내 실행 차례가 아니다.
	const FQuestAssignment* Assignment = Service->FindQuestAssignmentByNpc(Npc->NpcId, EQuestAssignmentState::Accepted);
	if (Assignment == nullptr) return EBTNodeResult::Failed;

	FBTDoQuestMemory* Mem = CastInstanceNodeMemory<FBTDoQuestMemory>(NodeMemory);
	Mem->QuestService = Service;
	// 포인터가 아니라 id만 들고 간다 — QuestAssignments가 바뀌면 위 포인터는 그 즉시 무효.
	Mem->AssignmentId = Assignment->AssignmentId;
	Mem->RemainingTime = QuestDuration;

	UE_LOG(LogTemp, Warning, TEXT("Do Quest Start. Assignment:%d"), Mem->AssignmentId)
	return EBTNodeResult::InProgress;
}

void UBTTask_DoQuest::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTDoQuestMemory* Mem = CastInstanceNodeMemory<FBTDoQuestMemory>(NodeMemory);

	Mem->RemainingTime -= DeltaSeconds;
	if (Mem->RemainingTime > 0.f) return;

	// 완료의 검증·기록은 소유자(QuestService)가 서버 권위로 한다 — 여기서 State를 직접 쓰지 않는다.
	// 그 사이 Assignment가 사라졌거나(NPC 소멸·포기) 상태가 어긋났으면 전이가 거절되고 태스크도 실패한다.
	UQuestService* Service = Mem->QuestService.Get();
	const bool bCleared = Service != nullptr && Service->ClearQuestAssignment(Mem->AssignmentId);

	UE_LOG(LogTemp, Warning, TEXT("Do Quest Finished. Assignment:%d Cleared:%d"), Mem->AssignmentId, bCleared)
	FinishLatentTask(OwnerComp, bCleared ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
}

uint16 UBTTask_DoQuest::GetInstanceMemorySize() const
{
	return sizeof(FBTDoQuestMemory);
}

void UBTTask_DoQuest::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
	// NodeMemory는 raw. 비POD 멤버(TWeakObjectPtr)를 위해 placement new로 초기화.
	new (NodeMemory) FBTDoQuestMemory();
}
