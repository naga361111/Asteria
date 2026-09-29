// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_DeleteQuest.h"

#include "AIController.h"
#include "NPC/AsteriaNpc.h"
#include "GameState/AsteriaGameState.h"
#include "GameState/Components/QuestService.h"

UBTTask_DeleteQuest::UBTTask_DeleteQuest()
{
	NodeName = TEXT("Delete Quest");

	// 실행별 상태가 없다 → NodeMemory·노드 인스턴스화를 쓰지 않는다(공유 인스턴스 그대로).
}

EBTNodeResult::Type UBTTask_DeleteQuest::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// BT는 서버에서만 도는 전제. 월드/NpcId는 OwnerComp의 AI 오너·폰에서 얻는다.
	AAIController* AICon = OwnerComp.GetAIOwner();
	AAsteriaNpc* Npc = AICon ? Cast<AAsteriaNpc>(AICon->GetPawn()) : nullptr;
	if (Npc == nullptr) return EBTNodeResult::Failed;

	AAsteriaGameState* GS = Npc->GetWorld()->GetGameState<AAsteriaGameState>();
	if (GS == nullptr) return EBTNodeResult::Failed;

	UQuestService* Service = GS->QuestService;
	if (Service == nullptr) return EBTNodeResult::Failed;

	// 떠나는 NPC가 든 것이 하나도 남지 않게 모든 진행 상태에서 찾는다. Settled는 전이 직후 제거되므로 대상이 아니다.
	// 퀘스트를 못 집고 떠나는 경우엔 한 번도 안 지워지고 Succeeded — 실패로 끝나면 뒤따르는 떠나기(Exit 이동·Despawn)가 막힌다.
	for (uint8 S = (uint8)EQuestAssignmentState::Assigned; S <= (uint8)EQuestAssignmentState::SettleConfirmed; ++S)
	{
		const EQuestAssignmentState State = (EQuestAssignmentState)S;
		while (const FQuestAssignment* Assignment = Service->FindQuestAssignmentByNpc(Npc->NpcId, State))
		{
			// 포인터가 아니라 id만 쓴다 — 삭제로 QuestAssignments가 바뀌면 위 포인터는 무효.
			const int32 AssignmentId = Assignment->AssignmentId;
			if (!Service->DeleteQuestAssignment(AssignmentId))
			{
				UE_LOG(LogTemp, Warning, TEXT("DeleteQuestAssignment failed. Assignment:%d"), AssignmentId)
				return EBTNodeResult::Failed;
			}

			UE_LOG(LogTemp, Warning, TEXT("Assignment:%d Deleted"), AssignmentId)
		}
	}

	return EBTNodeResult::Succeeded;
}
