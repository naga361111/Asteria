// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AsteriaTimeSubsystem.generated.h"

/** 게임 전역 시간. 시작(0=자정)부터 멈추지 않고 흐르는 누적 게임 분을 보유한다.
 *  월드마다 자동 생성 — World->GetSubsystem<UAsteriaTimeSubsystem>()로 접근.
 *  시/분/날짜는 이 누적값에서 파생한다(지금은 저장·Getter만). */
UCLASS()
class ASTERIA_API UAsteriaTimeSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	/** 시작부터 누적된 게임 분(0=자정). 시간의 유일한 진실. */
	double GetTimeMinutes() const { return CurrentMinutes; }

	/** 시작부터 흐른 게임 일수(0부터). */
	int32 GetDay() const;

	/** 현재 시각의 시(0~23). */
	int32 GetHour() const;

	/** 현재 시각의 분(0~59). */
	int32 GetMinute() const;

	// FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

private:
	/** 시작부터 누적된 게임 분. 유일한 시간 상태. */
	double CurrentMinutes = 0.0;

	/** 실제 1초당 흐르는 게임 분. 0.4 = 현실 1시간에 게임 24시간(1440분). */
	float Scale = 0.4f;
};
