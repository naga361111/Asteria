// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/AsteriaGameState.h"
#include "Quest/Quest.h"

void AAsteriaGameState::GenerateSystemQuestPulls()
{
	for (int i = 0; i < QuestPullsCount; ++i)
	{
		FQuest Quest = { FMath::RandRange(0, 100) };
		SystemQuestPulls.Add(Quest);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Quest Pulls Generated: %d"), QuestPullsCount);
}

void AAsteriaGameState::BeginPlay()
{
	Super::BeginPlay();
	
	GenerateSystemQuestPulls();
}
