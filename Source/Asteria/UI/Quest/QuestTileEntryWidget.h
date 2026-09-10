// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "QuestTileEntryWidget.generated.h"

class UTextBlock;

/**
 * TileView가 재사용하는 카드 한 칸. 데이터 바인딩 시 C++에서 직접 텍스트를 채운다.
 * WBP는 QuestIdText / QuestTypeText 위젯만 제공한다.
 */
UCLASS()
class ASTERIA_API UQuestTileEntryWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	// TileView가 이 칸에 item object를 바인딩할 때 호출
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> QuestIdText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> QuestTypeText;
};
