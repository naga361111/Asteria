// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Place/PlaceTypes.h"
#include "NpcActionSet.generated.h"

class UBehaviorTree;

/** 세트의 한 스텝 — 실행할 원자 BT + 그 스텝의 인자(목적지 지점 종류).
 *  에디터에서 인덱스마다 BT와 Point를 지정한다. */
USTRUCT(BlueprintType)
struct FActionStep
{
	GENERATED_BODY()

	/** 이 스텝에서 실행할 원자 BT. */
	UPROPERTY(EditAnywhere, Category="Action Step")
	TObjectPtr<UBehaviorTree> BT;

	/** 목적지 건물 카테고리. 런타임에 이 종류의 건물을 잡는다. */
	UPROPERTY(EditAnywhere, Category="Action Step")
	EBuildingType Category = EBuildingType::Restaurant;

	/** 이 스텝의 이동 목적지 지점 종류(단일). None이면 이동 안 함(대기 등). */
	UPROPERTY(EditAnywhere, Category="Action Step")
	EPlacePoint Point = EPlacePoint::None;
};

/** 한 행동 세트의 정의 — 순서대로 실행할 스텝 배열. 에디터에서 Data Asset으로 찍어 만든다.
 *  런타임에 runner가 이 배열을 받아 하나씩 실행. */
UCLASS(BlueprintType)
class ASTERIA_API UNpcActionSet : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 순서대로 실행할 스텝들(원자 BT + 인자). */
	UPROPERTY(EditAnywhere, Category="Action Set")
	TArray<FActionStep> Steps;
};
