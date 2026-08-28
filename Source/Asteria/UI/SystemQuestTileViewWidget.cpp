// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SystemQuestTileViewWidget.h"

#include "Components/TileView.h"
#include "Quest/Quest.h"
#include "GameState/AsteriaGameState.h"
#include "TileView/QuestEntryObject.h"

void USystemQuestTileViewWidget::NativeConstruct()
{
	
	const AAsteriaGameState* GS = GetWorld()->GetGameState<AAsteriaGameState>();
	const TArray<FQuest>& SystemQuestsPull = GS->QuestPulls;
	
	for (const FQuest& Quest : SystemQuestsPull)
	{
		UQuestEntryObject* EntryObject = NewObject<UQuestEntryObject>(this);
		EntryObject->Quest = Quest;
		
		TileView->AddItem(EntryObject);
	}
}
