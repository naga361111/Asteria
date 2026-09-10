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

	const FString TypeName = StaticEnum<EQuest>()->GetNameStringByValue(
		static_cast<int64>(Entry->Quest.QuestType));
	QuestTypeText->SetText(FText::FromString(TypeName));
}
