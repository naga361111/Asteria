// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_SelectQuest.h"

#include "AIController.h"
#include "NPC/AsteriaNpc.h"
#include "GameState/AsteriaGameState.h"
#include "GameState/Components/QuestService.h"

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

	// 무엇을 집을지만 고른다. 소유권 획득의 검증·기록은 ClaimQuest가 서버 권위로 한다.
	const int32 QuestId = QuestService->FindAvailableQuestId();
	if (QuestId == INDEX_NONE)
	{
		return EBTNodeResult::Failed;
	}

	if (QuestService->ClaimQuest(QuestId, { Npc->NpcId }) == INDEX_NONE)
	{
		return EBTNodeResult::Failed;
	}

	UE_LOG(LogTemp, Warning, TEXT("Claimed: %d"), QuestId)
	UE_LOG(LogTemp, Warning, TEXT("Claim: %d"), QuestService->Claims.Num())
	return EBTNodeResult::Succeeded;
}
