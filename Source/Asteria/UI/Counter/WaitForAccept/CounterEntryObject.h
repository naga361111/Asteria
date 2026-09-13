// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CounterEntryObject.generated.h"

/**
 * TileView의 item source 한 칸. 제출 상태인 Assignment 하나를 UObject로 승격시킨 view-model.
 *
 * 지금은 ID만 든다. QuestId·파티 같은 Assignment 본체 값을 칸에 그리려면
 * UQuestService::FindQuestAssignment로 역참조해 파생 필드를 채우면 된다
 * (UQuestEntryObject의 bAssigned와 같은 방식 — join은 칸이 아니라 컨테이너의 일).
 */
UCLASS()
class ASTERIA_API UCounterEntryObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 AssignmentId = INDEX_NONE;
};
