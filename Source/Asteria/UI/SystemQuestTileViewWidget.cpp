// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SystemQuestTileViewWidget.h"

#include "Components/TileView.h"
#include "Quest/Quest.h"
#include "GameState/AsteriaGameState.h"
#include "TileView/QuestEntryObject.h"

void USystemQuestTileViewWidget::NativeConstruct()
{
	AAsteriaGameState* GS = GetWorld()->GetGameState<AAsteriaGameState>();
	GS->OnQuestPullsChanged.AddUObject(this, &USystemQuestTileViewWidget::RefreshTiles);
	
	RefreshTiles();
}

void USystemQuestTileViewWidget::RefreshTiles()
{
	const AAsteriaGameState* GS = GetWorld()->GetGameState<AAsteriaGameState>();
	const TArray<FQuest>& SystemQuestsPull = GS->QuestPulls;
	
	TileView->ClearListItems();

	if (QuestType == EQuestType::System)
	{
		for (const FQuest& Quest : SystemQuestsPull)
		{
			if (Quest.bIsPosted) continue;

			UQuestEntryObject* EntryObject = NewObject<UQuestEntryObject>(this);
			EntryObject->Quest = Quest;
			TileView->AddItem(EntryObject);
		}
	}
	else
	{
		for (const FQuest& Quest : SystemQuestsPull)
		{
			if (!Quest.bIsPosted) continue;

			UQuestEntryObject* EntryObject = NewObject<UQuestEntryObject>(this);
			EntryObject->Quest = Quest;
			TileView->AddItem(EntryObject);
		}
	}
}
