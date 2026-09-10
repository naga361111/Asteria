// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Common/Quest.h"
#include "AsteriaGameState.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnQuestPullChanged);

/**
 *
 */
UCLASS()
class ASTERIA_API AAsteriaGameState : public AGameStateBase
{
	GENERATED_BODY()

	AAsteriaGameState();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	int32 QuestCount = 0;

public:
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_QuestPull,Category="Quest")
	TArray<FQuest> QuestPull;
	
	UFUNCTION()
	void OnRep_QuestPull();

	// QuestPull이 바뀌면 이걸 Broadcast (호출자 책임)
	FOnQuestPullChanged OnQuestPullChanged;
};
