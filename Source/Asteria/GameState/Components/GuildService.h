// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Common/Rank.h"
#include "GuildService.generated.h"

class UGuildReputationData;

// GuildFunds가 바뀌었음을 알리는 신호. 서버는 AddGuildFunds에서, 클라는 OnRep에서 Broadcast한다.
DECLARE_MULTICAST_DELEGATE(FOnGuildFundsChanged);

// GuildRank·GuildReputation이 바뀌었음을 알리는 신호. 서버는 AddGuildReputation에서, 클라는 OnRep에서 Broadcast한다.
DECLARE_MULTICAST_DELEGATE(FOnGuildReputationChanged);

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
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_GuildFunds, Category="Guild")
	int32 GuildFunds = 0;

	UFUNCTION()
	void OnRep_GuildFunds();

	// GuildFunds가 바뀌면 Broadcast. GuildFundsWidget이 구독한다.
	FOnGuildFundsChanged OnGuildFundsChanged;

	// 길드 등급. 쓰기는 서버 권위.
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_GuildReputation, Category="Guild")
	ERank GuildRank = ERank::F;

	// 길드 누적 명성. 쓰기는 서버 권위. 승급해도 초기화하지 않는다.
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_GuildReputation, Category="Guild")
	int32 GuildReputation = 0;

	// 등급과 명성은 함께 바뀌므로 같은 OnRep을 공유한다.
	UFUNCTION()
	void OnRep_GuildReputation();

	// GuildRank·GuildReputation이 바뀌면 Broadcast. GuildRankWidget이 구독한다.
	FOnGuildReputationChanged OnGuildReputationChanged;

	// 명성 표. BP_GameState의 컴포넌트 기본값에서 지정한다.
	UPROPERTY(EditDefaultsOnly, Category="Guild")
	TObjectPtr<UGuildReputationData> ReputationData;

	// 길드 잔액을 Amount만큼 늘린다. 실패(권위 없음/0 이하 금액) 시 false.
	bool AddGuildFunds(int32 Amount);

	// 누적 명성을 Amount만큼 늘리고, 다음 등급 기준을 넘으면 승급한다(연속 승급 가능, S에서 멈춤).
	// 실패(권위 없음/0 이하 명성) 시 false.
	bool AddGuildReputation(int32 Amount);

	// QuestRank 퀘스트를 정산했을 때 얻었을 명성만큼 누적 명성을 줄인다(0 아래로 내려가지 않음).
	// 누적 명성이 현재 등급 도달 기준 아래면 강등한다(연속 강등 가능, F에서 멈춤, 기준 항목이 없으면 강등만 멈춤).
	// 실패(권위 없음/명성 표 없음/표에 해당 등급 항목 없음) 시 false.
	bool LoseQuestReputation(ERank QuestRank);

	// 현 등급 도달 기준 → 다음 등급 기준 구간에서 누적 명성이 차지하는 비율(0~1).
	// S면 1, 표가 없거나 기준 항목이 없거나 구간 폭이 0 이하면 0.
	float GetReputationProgress() const;
};
