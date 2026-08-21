// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NpcActionDef.generated.h"

/** 효용 AI가 참조하는 욕구 종류. NeedsComponent의 4대 게이지와 1:1. */
UENUM(BlueprintType)
enum class ENpcNeed : uint8
{
	Survival,
	Safety,
	Duty,
	Social,
};

/** 행동 하나의 정의(DataTable 행). 기능은 없고, 어떤 욕구에 얼마나 반응하는지만 담는다.
 *  에디터에서 행을 추가/삭제해 행동을 늘리고 줄인다. */
USTRUCT(BlueprintType)
struct FNpcActionDef : public FTableRowBase
{
	GENERATED_BODY()

	/** 이 행동이 반응하는 욕구별 가중치. 0~N개 동적 지정. 비면 상수 행동(Idle 문턱). */
	UPROPERTY(EditAnywhere, Category="Action")
	TMap<ENpcNeed, float> NeedWeights;

	/** 상수 가산점. 가중치가 비어 있으면 이 값만으로 점수가 된다. */
	UPROPERTY(EditAnywhere, Category="Action")
	float BaseScore = 0.f;
};
