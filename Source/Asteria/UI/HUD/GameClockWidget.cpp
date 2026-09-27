// Fill out your copyright notice in the Description page of Project Settings.


#include "GameClockWidget.h"
#include "Components/TextBlock.h"
#include "GameState/AsteriaGameState.h"
#include "GameState/Components/GameClockService.h"

void UGameClockWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!ClockText)
	{
		return;
	}

	AAsteriaGameState* GS = GetWorld()->GetGameState<AAsteriaGameState>();
	if (!GS || !GS->GameClockService)
	{
		return;
	}

	const UGameClockService* Clock = GS->GameClockService;
	ClockText->SetText(FText::FromString(FString::Printf(TEXT("%d일차 %02d:%02d"), Clock->GetDay(), Clock->GetHour(), Clock->GetMinute())));
}
