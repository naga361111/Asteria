// Fill out your copyright notice in the Description page of Project Settings.


#include "SystemQuestTile.h"

#include "QuestEntryObject.h"
#include "Components/TextBlock.h"
#include "Player/AsteriaPlayer.h"

void USystemQuestTile::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	// BP OnListItemObjectSet 이벤트가 계속 발동하도록 인터페이스 기본 구현 호출.
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	Entry = Cast<UQuestEntryObject>(ListItemObject);
	QuestIdText->SetText(FText::AsNumber(Entry->Quest.QuestId));
	RcdRankText->SetText(UEnum::GetDisplayValueAsText(Entry->Quest.RecommendedRank));
}

void USystemQuestTile::NativeConstruct()
{
	QuestButton->OnClicked.AddDynamic(this, &USystemQuestTile::OnConfirmButtonClicked);
}

void USystemQuestTile::OnConfirmButtonClicked()
{
	if (Entry->QuestType == EQuestType::System)
	{
		GetOwningPlayerPawn<AAsteriaPlayer>()->Server_PostQuest(Entry->Quest.QuestId);
	}
	else
	{
		GetOwningPlayerPawn<AAsteriaPlayer>()->Server_UnpostQuest(Entry->Quest.QuestId);
	}
}
