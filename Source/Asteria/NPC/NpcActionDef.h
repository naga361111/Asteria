// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NpcActionDef.generated.h"

/** 효용 AI가 참조하는 고려값 종류. DT(NeedWeights·Multipliers) 드롭다운을 채운다.
 *  지금은 허기 하나. NeedsComponent 필드와는 별개 — 잇는 다리(스코어링)는 아직 없다. */
UENUM(BlueprintType)
enum class ENpcNeed : uint8
{
	Hunger,
};

class UNpcActionSet;

/** 행동 하나의 정의(DataTable 행). 어떤 욕구에 반응하는지(점수) + 무엇으로 실행되는지(BT).
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

	/** 이 행동을 실행하는 Set(원자 BT 배열). 선택되면 runner가 이 Set을 순서대로 실행. 비면 실행 없음(선택만). */
	UPROPERTY(EditAnywhere, Category="Action")
	TObjectPtr<UNpcActionSet> ActionSet;
};
