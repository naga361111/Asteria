// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "GuildMoneyWidget.generated.h"

/**
 * 
 */
UCLASS()
class ASTERIA_API UGuildMoneyWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
	void HandleMoneyChanged(int32 NewAmount);
	
public:
	UPROPERTY(EditDefaultsOnly,meta=(BindWidget))
	TObjectPtr<UTextBlock> MoneyText;
};
