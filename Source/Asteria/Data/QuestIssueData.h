// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Common/Rank.h"
#include "QuestIssueData.generated.h"

/**
 * 한 길드 등급에서의 퀘스트 묶음 발행·만료 설정. 단위는 전부 게임 분.
 */
USTRUCT()
struct FQuestIssueSettings
{
	GENERATED_BODY()

	// 발행 예정 시각 간격.
	UPROPERTY(EditAnywhere, meta=(ClampMin="1"))
	int32 BatchPeriodMinutes = 18;

	// 실제 발행이 예정 시각에서 앞뒤로 흔들리는 폭. 간격보다 크면 발행 순서가 뒤집힐 수 있다.
	UPROPERTY(EditAnywhere, meta=(ClampMin="0"))
	int32 BatchJitterMinutes = 3;

	// 한 묶음에 발행되는 퀘스트 수 범위.
	UPROPERTY(EditAnywhere, meta=(ClampMin="0"))
	int32 MinBatchSize = 3;
	UPROPERTY(EditAnywhere, meta=(ClampMin="0"))
	int32 MaxBatchSize = 4;

	// 퀘스트 제한 시간 범위. 지나면 아무도 안 집은 퀘스트는 게시판에서 사라진다.
	UPROPERTY(EditAnywhere, meta=(ClampMin="1"))
	int32 MinQuestLifetimeMinutes = 24;
	UPROPERTY(EditAnywhere, meta=(ClampMin="1"))
	int32 MaxQuestLifetimeMinutes = 48;
};

/**
 * 퀘스트 발행 표. 길드 등급별 묶음 발행 간격·크기와 퀘스트 제한 시간을 담는다.
 */
UCLASS()
class ASTERIA_API UQuestIssueData : public UDataAsset
{
	GENERATED_BODY()

public:
	// 새 에셋이 기본 표로 채워진 상태로 만들어지도록 기본값을 넣는다.
	UQuestIssueData();

	// 길드 등급 → 발행·만료 설정. 발행 시점의 길드 등급 항목을 쓴다.
	UPROPERTY(EditAnywhere, Category="Quest")
	TMap<ERank, FQuestIssueSettings> IssueSettingsByGuildRank;
};
