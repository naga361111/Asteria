// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameClockWidget.generated.h"

class UTextBlock;

/**
 * 게임 시간을 "N일차 HH:MM"으로 표시한다. 표시 전용 소비자.
 * 시간은 복제 이벤트가 없는 파생값이라 매 프레임 GameClockService를 폴링한다.
 */
UCLASS()
class ASTERIA_API UGameClockWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// WBP의 TextBlock과 이름 일치시킬 것
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTextBlock> ClockText;
};
