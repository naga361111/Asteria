// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestBoardWidget.h"
#include "QuestEntryObject.h"
#include "Components/TileView.h"
#include "GameState/AsteriaGameState.h"
#include "GameState/Components/QuestService.h"

void UQuestBoardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (AAsteriaGameState* GS = GetWorld()->GetGameState<AAsteriaGameState>())
	{
		if (UQuestService* Service = GS->QuestService)
		{
			Service->OnQuestPullChanged.AddUObject(this, &UQuestBoardWidget::RefreshQuests);
			RefreshQuests();
		}
	}
}

void UQuestBoardWidget::RefreshQuests()
{
	if (!QuestTileView)
	{
		return;
	}

	AAsteriaGameState* GS = GetWorld()->GetGameState<AAsteriaGameState>();
	if (!GS || !GS->QuestService)
	{
		return;
	}

	QuestTileView->ClearListItems();
	for (const FQuest& Quest : GS->QuestService->QuestPull)
	{
		UQuestEntryObject* Entry = NewObject<UQuestEntryObject>(this);
		Entry->Quest = Quest;
		QuestTileView->AddItem(Entry);
	}
}
