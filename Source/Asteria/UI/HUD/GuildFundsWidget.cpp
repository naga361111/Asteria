// Fill out your copyright notice in the Description page of Project Settings.


#include "GuildFundsWidget.h"
#include "Components/TextBlock.h"
#include "GameState/AsteriaGameState.h"
#include "GameState/Components/GuildService.h"

void UGuildFundsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (AAsteriaGameState* GS = GetWorld()->GetGameState<AAsteriaGameState>())
	{
		if (UGuildService* Service = GS->GuildService)
		{
			Service->OnGuildFundsChanged.AddUObject(this, &UGuildFundsWidget::RefreshFunds);
			RefreshFunds();
		}
	}
}

void UGuildFundsWidget::RefreshFunds()
{
	if (!FundsText)
	{
		return;
	}

	AAsteriaGameState* GS = GetWorld()->GetGameState<AAsteriaGameState>();
	if (!GS || !GS->GuildService)
	{
		return;
	}

	FundsText->SetText(FText::AsNumber(GS->GuildService->GuildFunds));
}
