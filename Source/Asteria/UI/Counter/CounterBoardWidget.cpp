// Fill out your copyright notice in the Description page of Project Settings.


#include "CounterBoardWidget.h"
#include "CounterEntryObject.h"
#include "Components/TileView.h"
#include "GameState/AsteriaGameState.h"
#include "GameState/Components/CounterService.h"

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
		BindToCounterService(GS);
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

	if (UCounterService* Service = BoundService.Get())
	{
		Service->OnSubmittedClaimsChanged.RemoveAll(this);
	}
	BoundService.Reset();

	Super::NativeDestruct();
}

void UCounterBoardWidget::HandleGameStateSet(AGameStateBase* NewGameState)
{
	BindToCounterService(Cast<AAsteriaGameState>(NewGameState));
}

void UCounterBoardWidget::BindToCounterService(AAsteriaGameState* GameState)
{
	UCounterService* Service = GameState ? GameState->CounterService : nullptr;
	if (!Service || BoundService.Get() == Service)
	{
		return;
	}

	Service->OnSubmittedClaimsChanged.AddUObject(this, &UCounterBoardWidget::RefreshSubmissions);
	BoundService = Service;

	RefreshSubmissions();
}

void UCounterBoardWidget::RefreshSubmissions()
{
	UCounterService* Service = BoundService.Get();
	if (!SubmissionTileView || !Service)
	{
		return;
	}

	SubmissionTileView->ClearListItems();
	for (const int32 ClaimId : Service->SubmittedClaimIds)
	{
		UCounterEntryObject* Entry = NewObject<UCounterEntryObject>(this);
		Entry->ClaimId = ClaimId;
		SubmissionTileView->AddItem(Entry);
	}
}
