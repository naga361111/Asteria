// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "NpcAIController.generated.h"

/**
 * 
 */
UCLASS()
class ASTERIA_API ANpcAIController : public AAIController
{
	GENERATED_BODY()
	
	ANpcAIController();
	
public:
	virtual void Tick(float DeltaSeconds) override;
	
	virtual void OnPossess(APawn* InPawn) override;
	
	UPROPERTY(EditDefaultsOnly, Category="BehaviorTree")
	TObjectPtr<UBehaviorTree> BehaviorTree;
	
	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float SlowdownRadius = 200.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float ArrivalSpeed = 50.0f;
	
	float BaseMaxSpeed = 0.0f;
};
