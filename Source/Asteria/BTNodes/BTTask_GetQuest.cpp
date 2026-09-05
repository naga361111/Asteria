// Fill out your copyright notice in the Description page of Project Settings.


#include "BTNodes/BTTask_GetQuest.h"

#include "AIController.h"
#include "GameState/AsteriaGameState.h"
#include "NPC/AsteriaNpc.h"

EBTNodeResult::Type UBTTask_GetQuest::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAsteriaGameState* AGS = GetWorld()->GetGameState<AAsteriaGameState>();
	AAsteriaNpc* OwnerNpc = OwnerComp.GetAIOwner()->GetPawn<AAsteriaNpc>();
	
	if (AGS == nullptr || OwnerNpc == nullptr) return EBTNodeResult::Failed;
	
	int32 QuestId = AGS->GetQuest(OwnerNpc->NpcRank);
	if (QuestId == -1)
	{
		return EBTNodeResult::Failed;   // 잡을 퀘스트 없음 → BT에서 다른 행동으로
	}
	OwnerNpc->AcceptedQuests.Add(QuestId);
	return EBTNodeResult::Succeeded;
}
