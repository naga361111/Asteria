// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TileView.h"
#include "Quest/QuestBoard.h"
#include "QuestBoardTileViewWidget.generated.h"

/**
 * 
 */
UCLASS()
class ASTERIA_API UQuestBoardTileViewWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTileView> TileView;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AQuestBoard> QuestBoard;
	
	virtual void NativeConstruct() override;
	
	void Refresh();
};
