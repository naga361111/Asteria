// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/AsteriaGameState.h"

#include "GameState/Components/QuestService.h"
#include "GameState/Components/CounterService.h"
#include "GameState/Components/GuildService.h"
#include "GameState/Components/GameClockService.h"
#include "GameState/Components/NpcSpawnService.h"
#include "GameState/Components/GuildShellService.h"

AAsteriaGameState::AAsteriaGameState()
{
	QuestService = CreateDefaultSubobject<UQuestService>(TEXT("QuestService"));
	CounterService = CreateDefaultSubobject<UCounterService>(TEXT("CounterService"));
	GuildService = CreateDefaultSubobject<UGuildService>(TEXT("GuildService"));
	GameClockService = CreateDefaultSubobject<UGameClockService>(TEXT("GameClockService"));
	NpcSpawnService = CreateDefaultSubobject<UNpcSpawnService>(TEXT("NpcSpawnService"));
	GuildShellService = CreateDefaultSubobject<UGuildShellService>(TEXT("GuildShellService"));
}
