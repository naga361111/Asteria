// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "NpcAIController.generated.h"

/** NPC의 뇌 자리. 지금은 빙의만 하고 가만히 있는다.
 *  나중에 효용 AI 평가 / Behavior Tree가 여기 얹힌다. */
UCLASS()
class ASTERIA_API ANpcAIController : public AAIController
{
	GENERATED_BODY()
};
