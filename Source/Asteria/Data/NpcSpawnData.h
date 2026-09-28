// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Common/Rank.h"
#include "NpcSpawnData.generated.h"

/**
 * NPC 방문 표. 길드 등급별 NPC 평균 방문 간격을 담는다.
 */
UCLASS()
class ASTERIA_API UNpcSpawnData : public UDataAsset
{
	GENERATED_BODY()

public:
	// 새 에셋이 기본 표로 채워진 상태로 만들어지도록 기본값을 넣는다.
	UNpcSpawnData();

	// 길드 등급 → NPC 평균 방문 간격(게임 분). 평균일 뿐 실제 간격은 매번 무작위다.
	// 이름난 길드일수록 자주 온다.
	UPROPERTY(EditAnywhere, Category="Spawn")
	TMap<ERank, float> SpawnIntervalMinutesByGuildRank;
};
