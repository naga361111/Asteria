// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Common/Rank.h"
#include "GuildReputationData.generated.h"

/**
 * 길드 명성 표. 퀘스트 정산으로 얻는 명성과 길드 등급 도달에 필요한 명성을 담는다.
 * 아직 사용처 없음 — 명성 적립·길드 승급이 붙을 때 참고용으로 먼저 저장해 둔 데이터다.
 */
UCLASS()
class ASTERIA_API UGuildReputationData : public UDataAsset
{
	GENERATED_BODY()

public:
	// 새 에셋이 기본 표로 채워진 상태로 만들어지도록 기본값을 넣는다.
	UGuildReputationData();

	// 퀘스트 등급 → 그 퀘스트를 정산했을 때 얻는 명성.
	UPROPERTY(EditAnywhere, Category="Reputation")
	TMap<ERank, int32> ReputationByQuestRank;

	// 길드 등급 → 그 등급에 도달하는 데 필요한 누적 명성.
	// 누적값이다 — 승급해도 명성을 0으로 초기화하지 않고 계속 쌓인 총량과 비교한다.
	UPROPERTY(EditAnywhere, Category="Reputation")
	TMap<ERank, int32> ReputationToReachRank;
};
