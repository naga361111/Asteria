// Fill out your copyright notice in the Description page of Project Settings.


#include "BTNodes/BTTask_DoQuest.h"

#include "Quest/Quest.h"
#include "AIController.h"
#include "GameState/AsteriaGameState.h"
#include "NPC/AsteriaNpc.h"

EBTNodeResult::Type UBTTask_DoQuest::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* TaskMemory)
{
	AAsteriaGameState* AGS = GetWorld()->GetGameState<AAsteriaGameState>();
	AAsteriaNpc* OwnerNpc = OwnerComp.GetAIOwner()->GetPawn<AAsteriaNpc>();

	if (AGS == nullptr || OwnerNpc == nullptr || OwnerNpc->AcceptedQuests.Num() == 0) return EBTNodeResult::Failed;

	int32 SelectedQuest = -1;
	for (int32 QuestId : OwnerNpc->AcceptedQuests)
	{
		FQuest* Found = AGS->QuestPulls.FindByPredicate([QuestId](const FQuest& Quest)
		{
			return Quest.QuestId == QuestId;
		});

		if (Found != nullptr && !Found->bIsCleared)
		{
			SelectedQuest = QuestId;
			break;
		}
	}

	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(
		Handle,
		FTimerDelegate::CreateLambda([this, &OwnerComp, AGS, SelectedQuest]()
		{
			AGS->ClearQuest(SelectedQuest);

			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}),
		WaitSeconds,
		false);

	return EBTNodeResult::InProgress;
}
