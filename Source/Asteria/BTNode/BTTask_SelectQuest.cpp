// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_SelectQuest.h"

#include "AIController.h"
#include "NPC/AsteriaNpc.h"
#include "GameState/AsteriaGameState.h"
#include "GameState/Components/QuestService.h"
#include "GameState/Components/GuildService.h"

EBTNodeResult::Type UBTTask_SelectQuest::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// BT는 서버에서만 도는 전제. 실제 소유·검증·기록은 QuestService가 한다.
	AAIController* AICon = OwnerComp.GetAIOwner();
	AAsteriaNpc* Npc = AICon ? Cast<AAsteriaNpc>(AICon->GetPawn()) : nullptr;
	if (!Npc)
	{
		return EBTNodeResult::Failed;
	}

	AAsteriaGameState* GameState = Npc->GetWorld()->GetGameState<AAsteriaGameState>();
	UQuestService* QuestService = GameState ? GameState->QuestService : nullptr;
	if (!QuestService)
	{
		return EBTNodeResult::Failed;
	}

	// 무엇을 집을지만 고른다. 소유권 획득의 검증·기록은 AssignQuest가 서버 권위로 한다.
	const int32 QuestId = QuestService->FindAvailableQuestId(Npc->NpcRnk);
	if (QuestId == INDEX_NONE)
	{
		// 집을 퀘스트가 없어 못 잡았다 — NPC 등급을 퀘스트 등급으로 보고 그만큼 길드 명성을 차감한다.
		if (UGuildService* GuildService = GameState->GuildService)
		{
			GuildService->LoseQuestReputation(Npc->NpcRnk);
		}
		return EBTNodeResult::Failed;
	}

	// 다른 NPC가 먼저 집어 거절된 경우는 의도한 루트가 아니므로 차감하지 않는다.
	if (QuestService->AssignQuest(QuestId, { Npc->NpcId }) == INDEX_NONE)
	{
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::Succeeded;
}
