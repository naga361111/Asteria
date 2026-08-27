// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SystemQuestTileViewWidget.h"

#include "QuestEntryObject.h"
#include "Components/TileView.h"
#include "Quest/Quest.h"
#include "GameState/AsteriaGameState.h"

void USystemQuestTileViewWidget::NativeConstruct()
{
	
	const AAsteriaGameState* GS = GetWorld()->GetGameState<AAsteriaGameState>();
	const TArray<FQuest>& SystemQuestsPull = GS->SystemQuestPulls;
	
	for (const FQuest& Quest : SystemQuestsPull)
	{
		UQuestEntryObject* EntryObject = NewObject<UQuestEntryObject>(this);
		EntryObject->QuestId = Quest.QuestId;
		
		TileView->AddItem(EntryObject);
	}
}
