// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SystemQuestTile.h"

#include "QuestEntryObject.h"
#include "Components/TextBlock.h"

void USystemQuestTile::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	// BP OnListItemObjectSet 이벤트가 계속 발동하도록 인터페이스 기본 구현 호출.
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	UQuestEntryObject* Entry = Cast<UQuestEntryObject>(ListItemObject);
	QuestIdText->SetText(FText::AsNumber(Entry->QuestId));
}

