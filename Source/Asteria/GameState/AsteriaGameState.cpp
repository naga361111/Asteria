// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/AsteriaGameState.h"

#include "GameState/Components/QuestService.h"

AAsteriaGameState::AAsteriaGameState()
{
	QuestService = CreateDefaultSubobject<UQuestService>(TEXT("QuestService"));
}
