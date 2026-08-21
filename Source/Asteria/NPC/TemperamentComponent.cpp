// Copyright Epic Games, Inc. All Rights Reserved.

#include "TemperamentComponent.h"
#include "Engine/DataTable.h"

UTemperamentComponent::UTemperamentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

TArray<FName> UTemperamentComponent::GetRowNames() const
{
	if (Table == nullptr)
	{
		return TArray<FName>();
	}

	// 이미 등록된 기질은 드롭다운에서 뺀다(중복 선택지 숨김).
	TArray<FName> Names = Table->GetRowNames();
	Names.RemoveAll([this](const FName& Name) { return Temperaments.Contains(Name); });
	return Names;
}
