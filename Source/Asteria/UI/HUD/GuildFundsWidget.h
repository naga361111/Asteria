// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GuildFundsWidget.generated.h"

class UTextBlock;

/**
 * 길드 잔액(GuildFunds)을 표시한다. 표시 전용 소비자.
 */
UCLASS()
class ASTERIA_API UGuildFundsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	// GuildService의 GuildFunds를 읽어 FundsText에 표시 (bind 대상 + 초기 prime)
	void RefreshFunds();

	// WBP의 TextBlock과 이름 일치시킬 것
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTextBlock> FundsText;
};
