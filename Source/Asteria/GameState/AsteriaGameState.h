// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "AsteriaGameState.generated.h"

struct FQuest;
/**
 * 
 */
UCLASS()
class ASTERIA_API AAsteriaGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "Quest")
	TArray<FQuest> SystemQuestPulls;
	
	UPROPERTY(EditDefaultsOnly, Category = "Quest", meta = (ClampMin = 1, ClampMax = 100, UIMin = 1, UIMax = 100))
	int32 QuestPullsCount;
	
	virtual void BeginPlay() override;
private:
	UFUNCTION()
	void GenerateSystemQuestPulls();
};
