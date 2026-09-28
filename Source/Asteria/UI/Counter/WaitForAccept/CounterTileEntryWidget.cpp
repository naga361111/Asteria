// Fill out your copyright notice in the Description page of Project Settings.


#include "CounterTileEntryWidget.h"
#include "CounterEntryObject.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Player/AsteriaPlayer.h"

void UCounterTileEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ButtonText)
	{
		ButtonText->OnClicked.AddDynamic(this, &UCounterTileEntryWidget::HandleAcceptClicked);
	}
}

void UCounterTileEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	const UCounterEntryObject* Entry = Cast<UCounterEntryObject>(ListItemObject);
	if (!Entry)
	{
		return;
	}

	AssignmentId = Entry->AssignmentId;
	AssignmentIdText->SetText(FText::AsNumber(AssignmentId));
	QuestIdText->SetText(FText::AsNumber(Entry->QuestId));

	// 컨펌 대기(Submitted)만 누를 수 있다. Accepted는 NPC 회수 대기라 비활성.
	State = Entry->State;
	ButtonText->SetIsEnabled(State == EQuestAssignmentState::Submitted);
}

void UCounterTileEntryWidget::HandleAcceptClicked()
{
	// 회수 대기 칸의 중복 컨펌 차단. 서버도 상태 검사로 거절하지만 요청 자체를 보내지 않는다.
	if (AssignmentId == INDEX_NONE || State != EQuestAssignmentState::Submitted)
	{
		return;
	}

	// 상태를 바꾸는 입력이라 클라에서 판정하지 않고 서버로 넘긴다.
	// 소유 액터(폰)를 거쳐야 Server RPC가 라우팅된다 — 위젯은 소유자가 없다.
	AAsteriaPlayer* Player = Cast<AAsteriaPlayer>(GetOwningPlayerPawn());
	if (Player == nullptr)
	{
		// OwningPlayer 없이 CreateWidget된 경우. 입력 경로가 통째로 끊긴다.
		UE_LOG(LogTemp, Warning, TEXT("CounterTile: no owning AsteriaPlayer. Assignment:%d"), AssignmentId)
		return;
	}

	Player->Server_AcceptQuestAssignment(AssignmentId);
}
