// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "NpcAIController.generated.h"

/** NPC의 뇌. 효용 AI 평가기(UUtilityBrainComponent)를 얹어 욕구로 행동을 고른다. */
UCLASS()
class ASTERIA_API ANpcAIController : public AAIController
{
	GENERATED_BODY()

public:
	ANpcAIController();

protected:
	/** 효용 AI 평가기. 욕구를 읽어 행동을 고른다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="NPC")
	TObjectPtr<class UUtilityBrainComponent> Brain;

	/** 고른 행동의 원자 BT 배열을 순서대로 실행할 재생기. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="NPC")
	TObjectPtr<class UActionRunnerComponent> ActionRunner;
};
