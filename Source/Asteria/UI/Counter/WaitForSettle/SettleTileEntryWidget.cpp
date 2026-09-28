// Fill out your copyright notice in the Description page of Project Settings.


#include "SettleTileEntryWidget.h"
#include "SettleEntryObject.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Player/AsteriaPlayer.h"

void USettleTileEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ButtonText)
	{
		ButtonText->OnClicked.AddDynamic(this, &USettleTileEntryWidget::HandleSettleClicked);
	}

	// 재사용 전에 WBP 원래 색을 한 번만 기억한다. 이후 바인딩에서 색을 덮어써도 원본이 남는다.
	if (StateBorder)
	{
		DefaultBorderColor = StateBorder->GetBrushColor();
	}
}

void USettleTileEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	const USettleEntryObject* Entry = Cast<USettleEntryObject>(ListItemObject);
	if (!Entry)
	{
		return;
	}

	AssignmentId = Entry->AssignmentId;
	AssignmentIdText->SetText(FText::AsNumber(AssignmentId));
	QuestIdText->SetText(FText::AsNumber(Entry->QuestId));

	// 정산 컨펌 대기(SubmitForSettled)만 누를 수 있다. SettleConfirmed는 NPC 수령 대기라 비활성.
	State = Entry->State;
	ButtonText->SetIsEnabled(State == EQuestAssignmentState::SubmitForSettled);

	// 수령 대기는 별도 색. 재사용 위젯이라 SubmitForSettled도 매번 원래 색으로 되돌린다.
	StateBorder->SetBrushColor(State == EQuestAssignmentState::SettleConfirmed
		? WaitingReceiveColor
		: DefaultBorderColor);
}

void USettleTileEntryWidget::HandleSettleClicked()
{
	// 수령 대기 칸의 중복 컨펌 차단. 서버도 상태 검사로 거절하지만 요청 자체를 보내지 않는다.
	if (AssignmentId == INDEX_NONE || State != EQuestAssignmentState::SubmitForSettled)
	{
		return;
	}

	// 상태를 바꾸는 입력이라 클라에서 판정하지 않고 서버로 넘긴다.
	// 소유 액터(폰)를 거쳐야 Server RPC가 라우팅된다 — 위젯은 소유자가 없다.
	AAsteriaPlayer* Player = Cast<AAsteriaPlayer>(GetOwningPlayerPawn());
	if (Player == nullptr)
	{
		// OwningPlayer 없이 CreateWidget된 경우. 입력 경로가 통째로 끊긴다.
		UE_LOG(LogTemp, Warning, TEXT("SettleTile: no owning AsteriaPlayer. Assignment:%d"), AssignmentId)
		return;
	}

	Player->Server_SettleQuestAssignment(AssignmentId);
}
