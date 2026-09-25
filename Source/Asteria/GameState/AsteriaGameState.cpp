// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/AsteriaGameState.h"

#include "GameState/Components/QuestService.h"
#include "GameState/Components/CounterService.h"
#include "Net/UnrealNetwork.h"

AAsteriaGameState::AAsteriaGameState()
{
	QuestService = CreateDefaultSubobject<UQuestService>(TEXT("QuestService"));
	CounterService = CreateDefaultSubobject<UCounterService>(TEXT("CounterService"));
}

void AAsteriaGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAsteriaGameState, GuildFunds);
}

bool AAsteriaGameState::AddGuildFunds(int32 Amount)
{
	if (!HasAuthority() || Amount <= 0)
	{
		return false;
	}

	GuildFunds += Amount;
	return true;
}
