// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_ReceiveQuest.h"

#include "AIController.h"
#include "NPC/AsteriaNpc.h"
#include "GameState/AsteriaGameState.h"
#include "GameState/Components/QuestService.h"
#include "GameState/Components/CounterService.h"

UBTTask_ReceiveQuest::UBTTask_ReceiveQuest()
{
	NodeName = TEXT("Receive Quest");

	// 실행별 상태가 없다 → NodeMemory·노드 인스턴스화를 쓰지 않는다(공유 인스턴스 그대로).
}

EBTNodeResult::Type UBTTask_ReceiveQuest::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	// 내가 회수할 Assignment = 파티에 내 NpcId가 들고 플레이어가 컨펌한(Accepted) 것.
	// 상태를 안 보면 컨펌 전(Submitted)이나 이미 회수한 것까지 집는다.
	const FQuestAssignment* Assignment = Service->FindQuestAssignmentByNpc(Npc->NpcId, EQuestAssignmentState::Accepted);
	if (Assignment == nullptr) return EBTNodeResult::Failed;

	// 포인터가 아니라 id만 쓴다 — 회수로 QuestAssignments가 바뀌면 위 포인터는 무효.
	const int32 AssignmentId = Assignment->AssignmentId;
	if (!Counter->ReceiveQuestAssignment(AssignmentId))
	{
		UE_LOG(LogTemp, Warning, TEXT("ReceiveQuestAssignment failed. Assignment:%d"), AssignmentId)
		return EBTNodeResult::Failed;
	}

	UE_LOG(LogTemp, Warning, TEXT("Assignment:%d Received"), AssignmentId)
	return EBTNodeResult::Succeeded;
}
