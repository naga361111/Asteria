// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_ReceiveSettle.h"

#include "AIController.h"
#include "NPC/AsteriaNpc.h"
#include "GameState/AsteriaGameState.h"
#include "GameState/Components/QuestService.h"
#include "GameState/Components/CounterService.h"

UBTTask_ReceiveSettle::UBTTask_ReceiveSettle()
{
	NodeName = TEXT("Receive Settle");

	// 실행별 상태가 없다 → NodeMemory·노드 인스턴스화를 쓰지 않는다(공유 인스턴스 그대로).
}

EBTNodeResult::Type UBTTask_ReceiveSettle::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	// 내가 수령할 Assignment = 파티에 내 NpcId가 들고 플레이어가 정산 컨펌한(SettleConfirmed) 것.
	// 상태를 안 보면 컨펌 전(SubmitForSettled)까지 집는다.
	const FQuestAssignment* Assignment = Service->FindQuestAssignmentByNpc(Npc->NpcId, EQuestAssignmentState::SettleConfirmed);
	if (Assignment == nullptr) return EBTNodeResult::Failed;

	// 포인터가 아니라 id만 쓴다 — 수령으로 QuestAssignments가 바뀌면 위 포인터는 무효.
	const int32 AssignmentId = Assignment->AssignmentId;
	if (!Counter->ReceiveSettleQuestAssignment(AssignmentId))
	{
		UE_LOG(LogTemp, Warning, TEXT("ReceiveSettleQuestAssignment failed. Assignment:%d"), AssignmentId)
		return EBTNodeResult::Failed;
	}

	UE_LOG(LogTemp, Warning, TEXT("Assignment:%d Settled"), AssignmentId)
	return EBTNodeResult::Succeeded;
}
