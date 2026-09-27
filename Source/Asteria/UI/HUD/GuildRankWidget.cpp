// Fill out your copyright notice in the Description page of Project Settings.


#include "GuildRankWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "GameState/AsteriaGameState.h"
#include "GameState/Components/GuildService.h"

void UGuildRankWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (AAsteriaGameState* GS = GetWorld()->GetGameState<AAsteriaGameState>())
	{
		if (UGuildService* Service = GS->GuildService)
		{
			Service->OnGuildReputationChanged.AddUObject(this, &UGuildRankWidget::RefreshRank);
			RefreshRank();
		}
	}
}

void UGuildRankWidget::RefreshRank()
{
	if (!RankText || !ReputationBar)
	{
		return;
	}

	AAsteriaGameState* GS = GetWorld()->GetGameState<AAsteriaGameState>();
	if (!GS || !GS->GuildService)
	{
		return;
	}

	RankText->SetText(StaticEnum<ERank>()->GetDisplayNameTextByValue(static_cast<int64>(GS->GuildService->GuildRank)));
	ReputationBar->SetPercent(GS->GuildService->GetReputationProgress());
}
