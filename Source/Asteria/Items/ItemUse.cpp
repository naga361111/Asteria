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

	// 적힌 변화량을 그대로 더한다(음수 = 감소). 범위는 NeedsComponent 규약대로 0~100.
	// 맵이 비어 있으면 아무 변화 없이 성공 — 행은 찾았으니 사용은 성립.
	for (const TPair<ENpcNeed, int32>& Delta : Effect->NeedDeltas)
	{
		int32& Value = Needs->Values.FindOrAdd(Delta.Key);
		Value = FMath::Clamp(Value + Delta.Value, 0, 100);
	}
	return true;
}
