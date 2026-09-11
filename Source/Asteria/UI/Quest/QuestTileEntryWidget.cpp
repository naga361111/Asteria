// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestTileEntryWidget.h"
#include "QuestEntryObject.h"
#include "Components/TextBlock.h"

void UQuestTileEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	const UQuestEntryObject* Entry = Cast<UQuestEntryObject>(ListItemObject);
	if (!Entry)
	{
		return;
	}

	QuestIdText->SetText(FText::AsNumber(Entry->Quest.QuestId));

	// QuestTypeText 위젯명은 WBP 바인딩 유지를 위해 그대로. 표시 내용은 파생 가용성.
	QuestTypeText->SetText(FText::FromString(Entry->bClaimed ? TEXT("Claimed") : TEXT("Available")));
}
