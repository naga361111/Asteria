// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GuildRankWidget.generated.h"

class UTextBlock;
class UProgressBar;

/**
 * 길드 등급(GuildRank)과 명성 진행률을 표시한다. 표시 전용 소비자.
 */
UCLASS()
class ASTERIA_API UGuildRankWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	// GuildService의 GuildRank·명성 진행률을 읽어 표시 (bind 대상 + 초기 prime)
	void RefreshRank();

	// WBP의 TextBlock과 이름 일치시킬 것
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTextBlock> RankText;

	// WBP의 ProgressBar와 이름 일치시킬 것
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UProgressBar> ReputationBar;
};
