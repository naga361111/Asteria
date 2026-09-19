// Fill out your copyright notice in the Description page of Project Settings.


#include "SettleBoardWidget.h"
#include "SettleEntryObject.h"
#include "Components/TileView.h"
#include "GameState/AsteriaGameState.h"
#include "GameState/Components/QuestService.h"

void USettleBoardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (AAsteriaGameState* GS = World->GetGameState<AAsteriaGameState>())
	{
		BindToQuestService(GS);
	}
	else
	{
		// 아직 복제 전. 도착하면 그때 붙는다.
		GameStateSetHandle = World->GameStateSetEvent.AddUObject(this, &USettleBoardWidget::HandleGameStateSet);
	}
}

void USettleBoardWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		if (GameStateSetHandle.IsValid())
		{
			World->GameStateSetEvent.Remove(GameStateSetHandle);
			GameStateSetHandle.Reset();
		}
	}

	if (UQuestService* Service = BoundService.Get())
	{
		Service->OnQuestAssignmentsChanged.RemoveAll(this);
	}
	BoundService.Reset();

	Super::NativeDestruct();
}

void USettleBoardWidget::HandleGameStateSet(AGameStateBase* NewGameState)
{
	BindToQuestService(Cast<AAsteriaGameState>(NewGameState));
}

void USettleBoardWidget::BindToQuestService(AAsteriaGameState* GameState)
{
	UQuestService* Service = GameState ? GameState->QuestService : nullptr;
	if (!Service || BoundService.Get() == Service)
	{
		return;
	}

	// 올라옴(Cleared→SubmitForSettled)도 내려감(→Settled)도 모두 Assignment의 State 전이라
	// QuestAssignments 변경 신호 하나면 충분하다.
	Service->OnQuestAssignmentsChanged.AddUObject(this, &USettleBoardWidget::RefreshSettlements);
	BoundService = Service;

	RefreshSettlements();
}

void USettleBoardWidget::RefreshSettlements()
{
	UQuestService* Service = BoundService.Get();
	if (!SettlementTileView || !Service)
	{
		return;
	}

	SettlementTileView->ClearListItems();
	for (const FQuestAssignment& Assignment : Service->QuestAssignments)
	{
		// 보상 대기 제출함 = QuestAssignments의 필터. 아직 수행 중인 Accepted, 창구에 안 올라온 Cleared,
		// 이미 정산된 Settled는 빠진다. QuestService::SettleQuestAssignments(사본 대기열)는 보지 않는다 —
		// 같은 사실의 진실원을 둘로 두지 않기 위해 State 하나만 읽는다.
		if (Assignment.State != EQuestAssignmentState::SubmitForSettled)
		{
			continue;
		}

		USettleEntryObject* Entry = NewObject<USettleEntryObject>(this);
		Entry->AssignmentId = Assignment.AssignmentId;
		Entry->QuestId = Assignment.QuestId;
		SettlementTileView->AddItem(Entry);
	}
}
