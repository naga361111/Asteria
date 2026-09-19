// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "CounterTileEntryWidget.generated.h"

class UButton;
class UTextBlock;

/**
 * 창구 제출함 TileView가 재사용하는 카드 한 칸. 데이터 바인딩 시 C++에서 직접 텍스트를 채운다.
 * WBP는 AssignmentIdText 위젯만 제공한다.
 *
 * QuestTileEntryWidget과 지금은 모양이 같지만 별도 클래스로 둔다 —
 * 창구 칸은 곧 컨펌·반려 입력(서버 RPC 경로)이 붙을 자리라 표현이 갈라진다.
 */
UCLASS()
class ASTERIA_API UCounterTileEntryWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	// 버튼 구독은 여기서 한 번만. TileView는 칸 위젯을 재사용하므로
	// NativeOnListItemObjectSet에서 붙이면 같은 핸들러가 중복으로 쌓인다.
	virtual void NativeOnInitialized() override;

	// TileView가 이 칸에 item object를 바인딩할 때 호출
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	// 수락 입력. 동적 델리게이트(OnClicked) 대상이라 UFUNCTION 필수.
	UFUNCTION()
	void HandleAcceptClicked();

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> AssignmentIdText;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> QuestIdText;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ButtonText;

private:
	// 이 칸이 지금 그리고 있는 대상. 재사용되는 위젯이라 바인딩될 때마다 갈린다.
	int32 AssignmentId = INDEX_NONE;
};
