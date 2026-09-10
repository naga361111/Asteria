// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestBoardWidget.h"
#include "QuestEntryObject.h"
#include "Components/TileView.h"
#include "GameState/AsteriaGameState.h"

void UQuestBoardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (AAsteriaGameState* GS = GetWorld()->GetGameState<AAsteriaGameState>())
	{
		GS->OnQuestPullChanged.AddUObject(this, &UQuestBoardWidget::RefreshQuests);
		RefreshQuests();
	}
}

void UQuestBoardWidget::RefreshQuests()
{
	if (AAsteriaGameState* GS = GetWorld()->GetGameState<AAsteriaGameState>())
	{
		SetQuests(GS->QuestPull);
	}
}

void UQuestBoardWidget::SetQuests(const TArray<FQuest>& Quests)
{
	if (!QuestTileView)
	{
		return;
	}

	QuestTileView->ClearListItems();
	for (const FQuest& Quest : Quests)
	{
		UQuestEntryObject* Entry = NewObject<UQuestEntryObject>(this);
		Entry->Quest = Quest;
		QuestTileView->AddItem(Entry);
	}
}
