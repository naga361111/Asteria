// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "AsteriaGameState.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnQuestPullsChanged)

struct FQuest;
/**
 * 
 */
UCLASS()
class ASTERIA_API AAsteriaGameState : public AGameStateBase
{
	GENERATED_BODY()
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	UPROPERTY(VisibleAnywhere, Category = "Quest", ReplicatedUsing=OnRep_QuestPulls)
	TArray<FQuest> QuestPulls;
	
	UFUNCTION()
	void OnRep_QuestPulls();
	
	UPROPERTY(EditDefaultsOnly, Category = "Quest", meta = (ClampMin = 1, ClampMax = 100, UIMin = 1, UIMax = 100))
	int32 QuestPullsCount = 1;
	
	virtual void BeginPlay() override;
	
	void PostQuest(int32 QuestId);
	void UnpostQuest(int32 QuestId);
	
	FOnQuestPullsChanged OnQuestPullsChanged;
	
	int32 GetQuest(); 
};
