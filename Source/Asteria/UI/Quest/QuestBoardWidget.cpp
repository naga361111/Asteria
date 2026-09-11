// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestBoardWidget.h"
#include "QuestEntryObject.h"
#include "Components/TileView.h"
#include "GameState/AsteriaGameState.h"
#include "GameState/Components/QuestService.h"

void UQuestBoardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (AAsteriaGameState* GS = GetWorld()->GetGameState<AAsteriaGameState>())
	{
		if (UQuestService* Service = GS->QuestService)
		{
			// 표시 상태는 QuestPull(무엇) + Claims(집힘 여부)의 join이라 둘 다 구독한다.
			// 현재 흐름에선 QuestPull은 정적이고 실제 갱신은 Claims 변경에서 온다.
			Service->OnQuestPullChanged.AddUObject(this, &UQuestBoardWidget::RefreshQuests);
			Service->OnClaimsChanged.AddUObject(this, &UQuestBoardWidget::RefreshQuests);
			RefreshQuests();
		}
	}
}

void UQuestBoardWidget::RefreshQuests()
{
	if (!QuestTileView)
	{
		return;
	}

	AAsteriaGameState* GS = GetWorld()->GetGameState<AAsteriaGameState>();
	if (!GS || !GS->QuestService)
	{
		return;
	}

	UQuestService* Service = GS->QuestService;
	QuestTileView->ClearListItems();
	for (const FQuest& Quest : Service->QuestPull)
	{
		UQuestEntryObject* Entry = NewObject<UQuestEntryObject>(this);
		Entry->Quest = Quest;
		Entry->bClaimed = Service->IsQuestClaimed(Quest.QuestId);
		QuestTileView->AddItem(Entry);
	}
}
