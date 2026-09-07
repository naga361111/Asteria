// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/GuildMoneyWidget.h"

#include "GameState/AsteriaGameState.h"

void UGuildMoneyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	AAsteriaGameState* AGS = GetWorld()->GetGameState<AAsteriaGameState>();
	
	if (AGS != nullptr)
	{
		AGS->OnGuildMoneyChanged.AddUObject(this, &UGuildMoneyWidget::HandleMoneyChanged);
		HandleMoneyChanged(AGS->GuildMoney);
	}
}

void UGuildMoneyWidget::HandleMoneyChanged(int32 NewAmount)
{
	MoneyText->SetText(FText::AsNumber(NewAmount));
}
