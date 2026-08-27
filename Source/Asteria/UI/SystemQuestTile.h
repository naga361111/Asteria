// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Components/Button.h"
#include "SystemQuestTile.generated.h"

/**
 * 
 */
UCLASS()
class ASTERIA_API USystemQuestTile : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
	virtual void NativeConstruct() override;

public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> QuestIdText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> QuestButton;
	
	UFUNCTION()
	void OnConfirmButtonClicked();
};
