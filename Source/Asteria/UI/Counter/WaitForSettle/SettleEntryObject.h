// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SettleEntryObject.generated.h"

/**
 * 보상 대기 제출함 TileView의 item source 한 칸. State==SubmitForSettled인 Assignment 하나의 view-model.
 *
 * 값이 UCounterEntryObject와 같아도 타입은 나눈다 — 칸 위젯이 Cast로 자기 항목만 받아들이는 구조라
 * 타입이 곧 "어느 제출함의 칸인가"의 경계다. 같은 타입을 쓰면 수주함 칸에 정산 항목이 꽂혀도 통과한다.
 *
 * 파티 같은 나머지 본체 값을 더 그리려면 같은 방식으로 필드를 늘린다
 * (join은 칸이 아니라 컨테이너의 일 — UCounterEntryObject와 같은 규칙).
 */
UCLASS()
class ASTERIA_API USettleEntryObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 AssignmentId = INDEX_NONE;

	UPROPERTY()
	int32 QuestId = INDEX_NONE;
};
