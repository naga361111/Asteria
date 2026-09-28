// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_SubmitSettle.h"

#include "AIController.h"
#include "NPC/AsteriaNpc.h"
#include "GameState/AsteriaGameState.h"
#include "GameState/Components/QuestService.h"
#include "GameState/Components/CounterService.h"

UBTTask_SubmitSettle::UBTTask_SubmitSettle()
{
	NodeName = TEXT("Submit Settle");

	// 실행별 상태가 없다 → NodeMemory·노드 인스턴스화를 쓰지 않는다(공유 인스턴스 그대로).
}

EBTNodeResult::Type UBTTask_SubmitSettle::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	// 내가 올릴 Assignment = 파티에 내 NpcId가 들고 수행을 마친(Cleared) 것.
	// 상태를 안 보면 수행 중인 것·이미 올린 것까지 집는다.
	const FQuestAssignment* Assignment = Service->FindQuestAssignmentByNpc(Npc->NpcId, EQuestAssignmentState::Cleared);
	if (Assignment == nullptr) return EBTNodeResult::Failed;

	// 포인터가 아니라 id만 쓴다 — 제출로 QuestAssignments가 바뀌면 위 포인터는 무효.
	const int32 AssignmentId = Assignment->AssignmentId;
	if (!Counter->SubmitForSettleQuestAssignment(AssignmentId))
	{
		UE_LOG(LogTemp, Warning, TEXT("SubmitForSettleQuestAssignment failed. Assignment:%d"), AssignmentId)
		return EBTNodeResult::Failed;
	}

	UE_LOG(LogTemp, Warning, TEXT("Assignment:%d Submitted For Settle"), AssignmentId)
	return EBTNodeResult::Succeeded;
}
