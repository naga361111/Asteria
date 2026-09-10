// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/AsteriaGameState.h"

#include "Net/UnrealNetwork.h"

AAsteriaGameState::AAsteriaGameState()
{
	for (int i = 0; i < 50; ++i)
	{
		FQuest Quest = {QuestCount, EQuest::Generated};
		QuestPull.Add(Quest);
		
		QuestCount++;
	}
}

void AAsteriaGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(AAsteriaGameState, QuestPull);
}

void AAsteriaGameState::OnRep_QuestPull()
{
	OnQuestPullChanged.Broadcast();
}
