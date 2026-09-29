// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_ReturnQuest.h"

#include "AIController.h"
#include "NPC/AsteriaNpc.h"
#include "GameState/AsteriaGameState.h"
#include "GameState/Components/QuestService.h"

UBTTask_ReturnQuest::UBTTask_ReturnQuest()
{
	NodeName = TEXT("Return Quest");

	// 실행별 상태가 없다 → NodeMemory·노드 인스턴스화를 쓰지 않는다(공유 인스턴스 그대로).
}

EBTNodeResult::Type UBTTask_ReturnQuest::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// BT는 서버에서만 도는 전제. 월드/NpcId는 OwnerComp의 AI 오너·폰에서 얻는다.
	AAIController* AICon = OwnerComp.GetAIOwner();
	AAsteriaNpc* Npc = AICon ? Cast<AAsteriaNpc>(AICon->GetPawn()) : nullptr;
	if (Npc == nullptr) return EBTNodeResult::Failed;

	AAsteriaGameState* GS = Npc->GetWorld()->GetGameState<AAsteriaGameState>();
	if (GS == nullptr) return EBTNodeResult::Failed;

	UQuestService* Service = GS->QuestService;
	if (Service == nullptr) return EBTNodeResult::Failed;

	// 내가 반환할 Assignment = 아직 창구에서 회수하기 전(수주 단계)인 것. Received 이후는 수행·정산 중이라 대상이 아니다.
	const FQuestAssignment* Assignment = Service->FindQuestAssignmentByNpc(Npc->NpcId, EQuestAssignmentState::Submitted);
	if (Assignment == nullptr) Assignment = Service->FindQuestAssignmentByNpc(Npc->NpcId, EQuestAssignmentState::Assigned);
	if (Assignment == nullptr) Assignment = Service->FindQuestAssignmentByNpc(Npc->NpcId, EQuestAssignmentState::Accepted);

	// 퀘스트를 못 집고 대기하다 떠나는 경우 — 실패로 끝나면 뒤따르는 떠나기(Exit 이동·Despawn)가 막힌다.
	if (Assignment == nullptr) return EBTNodeResult::Succeeded;

	// 포인터가 아니라 id만 쓴다 — 반환으로 QuestAssignments가 바뀌면 위 포인터는 무효.
	const int32 AssignmentId = Assignment->AssignmentId;
	if (!Service->ReturnQuestAssignment(AssignmentId))
	{
		UE_LOG(LogTemp, Warning, TEXT("ReturnQuestAssignment failed. Assignment:%d"), AssignmentId)
		return EBTNodeResult::Failed;
	}

	UE_LOG(LogTemp, Warning, TEXT("Assignment:%d Returned"), AssignmentId)
	return EBTNodeResult::Succeeded;
}
