// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SystemQuestTileViewWidget.generated.h"

UENUM(BlueprintType)
enum class EQuestType : uint8
{
	System,
	Post,
};

class UTileView;
/**
 * 
 */
UCLASS()
class ASTERIA_API USystemQuestTileViewWidget : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTileView> TileView;
	
	virtual void NativeConstruct() override;
	
public:
	UPROPERTY(EditDefaultsOnly, Category="Quest")
	EQuestType QuestType;
	
	void RefreshTiles();
};
