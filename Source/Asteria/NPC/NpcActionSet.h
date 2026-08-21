// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NpcActionSet.generated.h"

class UBehaviorTree;

/** 한 행동 세트의 정의 — 순서대로 실행할 원자 BT 배열. 에디터에서 Data Asset으로 찍어 만든다.
 *  런타임에 runner가 이 배열을 받아 하나씩 실행(그 배선은 아직 없음). */
UCLASS(BlueprintType)
class ASTERIA_API UNpcActionSet : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 순서대로 실행할 원자 BT들. */
	UPROPERTY(EditAnywhere, Category="Action Set")
	TArray<TObjectPtr<UBehaviorTree>> Behaviors;
};
