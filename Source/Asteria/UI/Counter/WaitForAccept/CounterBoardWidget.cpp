// Fill out your copyright notice in the Description page of Project Settings.


#include "CounterBoardWidget.h"
#include "CounterEntryObject.h"
#include "Components/TileView.h"
#include "GameState/AsteriaGameState.h"
#include "GameState/Components/QuestService.h"

void UCounterBoardWidget::NativeConstruct()
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
		GameStateSetHandle = World->GameStateSetEvent.AddUObject(this, &UCounterBoardWidget::HandleGameStateSet);
	}
}

void UCounterBoardWidget::NativeDestruct()
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

void UCounterBoardWidget::HandleGameStateSet(AGameStateBase* NewGameState)
{
	BindToQuestService(Cast<AAsteriaGameState>(NewGameState));
}

void UCounterBoardWidget::BindToQuestService(AAsteriaGameState* GameState)
{
	UQuestService* Service = GameState ? GameState->QuestService : nullptr;
	if (!Service || BoundService.Get() == Service)
	{
		return;
	}

	// 제출·수락은 모두 Assignment의 State 전이라 QuestAssignments 변경 신호 하나면 충분하다.
	Service->OnQuestAssignmentsChanged.AddUObject(this, &UCounterBoardWidget::RefreshSubmissions);
	BoundService = Service;

	RefreshSubmissions();
}

void UCounterBoardWidget::RefreshSubmissions()
{
	UQuestService* Service = BoundService.Get();
	if (!SubmissionTileView || !Service)
	{
		return;
	}

	SubmissionTileView->ClearListItems();
	for (const FQuestAssignment& Assignment : Service->QuestAssignments)
	{
		// 제출함 = QuestAssignments의 필터. 창구에 아직 도착 안 한 Assigned, 이미 끝난 Accepted는 빠진다.
		if (Assignment.State != EQuestAssignmentState::Submitted)
		{
			continue;
		}

		UCounterEntryObject* Entry = NewObject<UCounterEntryObject>(this);
		Entry->AssignmentId = Assignment.AssignmentId;
		Entry->QuestId = Assignment.QuestId;
		SubmissionTileView->AddItem(Entry);
	}
}
