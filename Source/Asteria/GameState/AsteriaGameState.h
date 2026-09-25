// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "AsteriaGameState.generated.h"

class UCounterService;
class UQuestService;

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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 길드 공통 지갑 잔액. 쓰기는 서버 권위(AddGuildFunds), 클라는 복제된 값을 읽기만 한다.
	UPROPERTY(VisibleAnywhere, Replicated, Category="Guild")
	int32 GuildFunds = 0;

	// 길드 잔액을 Amount만큼 늘린다. 실패(권위 없음/0 이하 금액) 시 false.
	bool AddGuildFunds(int32 Amount);
};
