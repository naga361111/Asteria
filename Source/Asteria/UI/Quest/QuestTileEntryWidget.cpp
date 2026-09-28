// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestTileEntryWidget.h"
#include "QuestEntryObject.h"
#include "Components/TextBlock.h"
#include "GameState/Components/GameClockService.h"

void UQuestTileEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	const UQuestEntryObject* Entry = Cast<UQuestEntryObject>(ListItemObject);
	if (!Entry)
	{
		return;
	}

	QuestIdText->SetText(FText::AsNumber(Entry->Quest.QuestId));
	
	QuestRnk->SetText(StaticEnum<ERank>()->GetDisplayNameTextByValue(static_cast<int64>(Entry->Quest.QuestRnk)));

	// QuestTypeText 위젯명은 WBP 바인딩 유지를 위해 그대로. 표시 내용은 파생 가용성.
	QuestTypeText->SetText(FText::FromString(Entry->bAssigned ? TEXT("Assigned") : TEXT("Available")));

	// 집힌 퀘스트는 만료되지 않으므로 시각을 비운다
	ExpireTimeText->SetText(Entry->bAssigned ? FText::GetEmpty() : UGameClockService::FormatGameMinutes(Entry->Quest.ExpireGameMinute));
}
