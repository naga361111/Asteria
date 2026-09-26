// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "AsteriaGameState.generated.h"

class UCounterService;
class UQuestService;
class UGuildService;
class UGameClockService;

/**
 *
 */
UCLASS()
class ASTERIA_API AAsteriaGameState : public AGameStateBase
{
	GENERATED_BODY()

	AAsteriaGameState();

public:
	UPROPERTY(VisibleAnywhere, Category="Quest")
	TObjectPtr<UQuestService> QuestService;

	UPROPERTY(VisibleAnywhere, Category="Quest")
	TObjectPtr<UCounterService> CounterService;

	UPROPERTY(VisibleAnywhere, Category="Guild")
	TObjectPtr<UGuildService> GuildService;

	UPROPERTY(VisibleAnywhere, Category="Time")
	TObjectPtr<UGameClockService> GameClockService;
};
