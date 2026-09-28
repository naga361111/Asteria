// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Common/Quest.h"
#include "CounterEntryObject.generated.h"

/**
 * TileView의 item source 한 칸. Submitted 또는 Accepted 상태인 Assignment 하나를 UObject로 승격시킨 view-model.
 *
 * 파티 같은 나머지 Assignment 본체 값을 더 그리려면 같은 방식으로 필드를 늘리면 된다
 * (join은 칸이 아니라 컨테이너의 일 — 컨테이너가 Assignment를 돌며 채운다).
 */
UCLASS()
class ASTERIA_API UCounterEntryObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 AssignmentId = INDEX_NONE;

	UPROPERTY()
	int32 QuestId = INDEX_NONE;

	// 이 칸이 그리는 Assignment의 상태(Submitted 또는 Accepted). 칸 위젯이 컨펌 버튼 활성 여부를 가른다.
	UPROPERTY()
	EQuestAssignmentState State = EQuestAssignmentState::Submitted;
};
