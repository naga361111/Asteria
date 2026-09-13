// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/AsteriaGameState.h"

#include "GameState/Components/QuestService.h"
#include "GameState/Components/CounterService.h"

AAsteriaGameState::AAsteriaGameState()
{
	QuestService = CreateDefaultSubobject<UQuestService>(TEXT("QuestService"));
	CounterService = CreateDefaultSubobject<UCounterService>(TEXT("CounterService"));
}
