// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Common/Rank.h"
#include "GuildService.generated.h"

class UGuildReputationData;

/**
 * 길드 자체의 상태(자금·등급)를 소유한다. 쓰기는 서버 권위, 클라는 복제된 값을 읽기만 한다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ASTERIA_API UGuildService : public UActorComponent
{
	GENERATED_BODY()

public:
	UGuildService();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 길드 공통 지갑 잔액. 쓰기는 서버 권위(AddGuildFunds), 클라는 복제된 값을 읽기만 한다.
	UPROPERTY(VisibleAnywhere, Replicated, Category="Guild")
	int32 GuildFunds = 0;

	// 길드 등급. 쓰기는 서버 권위.
	UPROPERTY(VisibleAnywhere, Replicated, Category="Guild")
	ERank GuildRank = ERank::F;

	// 길드 누적 명성. 쓰기는 서버 권위. 승급해도 초기화하지 않는다.
	UPROPERTY(VisibleAnywhere, Replicated, Category="Guild")
	int32 GuildReputation = 0;

	// 명성 표. BP_GameState의 컴포넌트 기본값에서 지정한다.
	UPROPERTY(EditDefaultsOnly, Category="Guild")
	TObjectPtr<UGuildReputationData> ReputationData;

	// 길드 잔액을 Amount만큼 늘린다. 실패(권위 없음/0 이하 금액) 시 false.
	bool AddGuildFunds(int32 Amount);

	// 누적 명성을 Amount만큼 늘리고, 다음 등급 기준을 넘으면 승급한다(연속 승급 가능, S에서 멈춤).
	// 실패(권위 없음/0 이하 명성) 시 false.
	bool AddGuildReputation(int32 Amount);
};
