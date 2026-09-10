// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Common/Quest.h"
#include "AsteriaGameState.generated.h"

/**
 *
 */
UCLASS()
class ASTERIA_API AAsteriaGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, Category="Quest")
	TArray<FQuest> QuestPull;
};
