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

class UBehaviorTree;

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

	/** 이 행동을 실행하는 BT. 선택되면 컨트롤러가 이 트리를 돌린다. 비면 실행 없음(선택만). */
	UPROPERTY(EditAnywhere, Category="Action")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	/** 이동 BT가 읽을 목적지(월드 좌표). ponytail: 임시 하드 좌표. step 2에서 Blackboard/런타임 해석으로 이사. */
	UPROPERTY(EditAnywhere, Category="Action")
	FVector TargetLocation = FVector::ZeroVector;
};
