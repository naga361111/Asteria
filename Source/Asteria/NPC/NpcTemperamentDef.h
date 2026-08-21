// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NpcActionDef.h" // ENpcNeed
#include "NpcTemperamentDef.generated.h"

/** 기질 하나의 정의(DataTable 행). 세상을 보는 렌즈 — 욕구별 영구 배율.
 *  행동 가중치(덧셈)와 달리 이건 곱셈이라, 없는 욕구 = ×1.0(무편향)이다. */
USTRUCT(BlueprintType)
struct FNpcTemperamentDef : public FTableRowBase
{
	GENERATED_BODY()

	/** 이 기질이 각 욕구에 거는 배율. 비면 평범(전부 ×1.0). */
	UPROPERTY(EditAnywhere, Category="Temperament")
	TMap<ENpcNeed, float> Multipliers;
};
