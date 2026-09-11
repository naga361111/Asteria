// Fill out your copyright notice in the Description page of Project Settings.


#include "CounterTileEntryWidget.h"
#include "CounterEntryObject.h"
#include "Components/TextBlock.h"

void UCounterTileEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	const UCounterEntryObject* Entry = Cast<UCounterEntryObject>(ListItemObject);
	if (!Entry)
	{
		return;
	}

	ClaimIdText->SetText(FText::AsNumber(Entry->ClaimId));
}
