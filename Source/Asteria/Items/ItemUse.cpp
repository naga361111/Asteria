// Copyright Epic Games, Inc. All Rights Reserved.

#include "ItemUse.h"

#include "ItemEffectDef.h"
#include "NPC/NpcActionDef.h" // ENpcNeed
#include "NPC/NeedsComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"

bool UseItem(const UDataTable* EffectTable, FName ItemId, AActor* User)
{
	if (EffectTable == nullptr || User == nullptr)
	{
		return false;
	}

	const FItemEffectDef* Effect = EffectTable->FindRow<FItemEffectDef>(ItemId, TEXT("UseItem"));
	if (Effect == nullptr)
	{
		return false;
	}

	UNeedsComponent* Needs = User->FindComponentByClass<UNeedsComponent>();
	if (Needs == nullptr)
	{
		return false;
	}

	// 허기 회복 = Hunger 감소(0 clamp). 상한은 NeedsComponent 규약대로 100.
	int32& Hunger = Needs->Values.FindOrAdd(ENpcNeed::Hunger);
	Hunger = FMath::Clamp(Hunger - Effect->HungerRestore, 0, 100);
	return true;
}
