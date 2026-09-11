// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "CounterTileEntryWidget.generated.h"

class UTextBlock;

/**
 * 창구 제출함 TileView가 재사용하는 카드 한 칸. 데이터 바인딩 시 C++에서 직접 텍스트를 채운다.
 * WBP는 ClaimIdText 위젯만 제공한다.
 *
 * QuestTileEntryWidget과 지금은 모양이 같지만 별도 클래스로 둔다 —
 * 창구 칸은 곧 컨펌·반려 입력(서버 RPC 경로)이 붙을 자리라 표현이 갈라진다.
 */
UCLASS()
class ASTERIA_API UCounterTileEntryWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	// TileView가 이 칸에 item object를 바인딩할 때 호출
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ClaimIdText;
};
