// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/AsteriaGameState.h"
#include "Quest/Quest.h"

void AAsteriaGameState::BeginPlay()
{
	Super::BeginPlay();

	for (int i = 0; i < QuestPullsCount; ++i)
	{
		FQuest Quest = {FMath::RandRange(0, 100), false};
		QuestPulls.Add(Quest);
	}
	OnQuestPullsChanged.Broadcast();
}

void AAsteriaGameState::PostQuest(int32 QuestId)
{
	FQuest* Found  = QuestPulls.FindByPredicate([QuestId](const FQuest& Quest) { return Quest.QuestId == QuestId; });
	if (Found != nullptr)
	{
		Found->bIsPosted = true;
	}
	OnQuestPullsChanged.Broadcast();
}
