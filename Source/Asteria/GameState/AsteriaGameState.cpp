// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/AsteriaGameState.h"

AAsteriaGameState::AAsteriaGameState()
{
	for (int i = 0; i < 50; ++i)
	{
		FQuest Quest = {QuestCount, EQuest::Generated};
		QuestPull.Add(Quest);
		
		QuestCount++;
	}
}
