// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Common/Quest.h"
#include "QuestBoardWidget.generated.h"

class UTileView;

/**
 * QuestPull을 TileView로 표시하는 컨테이너. 표시 전용 소비자.
 */
UCLASS()
class ASTERIA_API UQuestBoardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// QuestPull(struct 배열)을 받아 TileView 갱신
	void SetQuests(const TArray<FQuest>& Quests);

protected:
	virtual void NativeConstruct() override;

	// GameState의 QuestPull을 읽어 다시 그림 (bind 대상 + 초기 prime)
	void RefreshQuests();

	// WBP의 TileView와 이름 일치시킬 것
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTileView> QuestTileView;
};
