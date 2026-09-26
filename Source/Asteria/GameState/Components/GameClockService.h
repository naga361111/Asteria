// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameClockService.generated.h"

/**
 * 게임 시간(일/시/분)을 계산한다. 따로 저장·복제하는 상태는 없고,
 * GameState가 이미 복제하는 서버 월드 시간에서 파생하므로 서버·클라가 같은 값을 읽는다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ASTERIA_API UGameClockService : public UActorComponent
{
	GENERATED_BODY()

public:
	UGameClockService();

	// 게임 시작부터 흐른 총 게임 분(시작 시각 포함). 오너가 GameState가 아니면 0.
	int32 GetGameMinutes() const;

	// 현재 일. 1일부터 시작.
	int32 GetDay() const;

	// 현재 시(0~23).
	int32 GetHour() const;

	// 현재 분(0~59).
	int32 GetMinute() const;
};
