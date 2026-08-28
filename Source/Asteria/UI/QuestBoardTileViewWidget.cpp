// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/QuestBoardTileViewWidget.h"

#include "QuestEntryObject.h"
#include "Quest/Quest.h"

void UQuestBoardTileViewWidget::NativeConstruct()
{
}

void UQuestBoardTileViewWidget::Refresh()
{
	TileView->ClearListItems();
	
	const TArray<FQuest>& QuestsPull = QuestBoard->QuestsPull;
	for (const FQuest& Quest : QuestsPull)
	{
		UQuestEntryObject* EntryObject = NewObject<UQuestEntryObject>(this);
		EntryObject->Quest = Quest;
		
		TileView->AddItem(EntryObject);
	}
	
}
