// Fill out your copyright notice in the Description page of Project Settings.


#include "GameClockService.h"

#include "GameFramework/GameStateBase.h"

namespace
{
	// 현실 1초당 흐르는 게임 분.
	constexpr float GameMinutesPerRealSecond = 1.f;

	// 게임 시작 시각(1일차 08:00)을 게임 분으로.
	constexpr int32 StartGameMinutes = 8 * 60;

	constexpr int32 MinutesPerHour = 60;
	constexpr int32 MinutesPerDay = 24 * MinutesPerHour;
}

UGameClockService::UGameClockService()
{
	// 복제할 상태가 없다 — 시간은 GameState가 복제하는 서버 월드 시간에서 파생한다.
	SetIsReplicatedByDefault(false);
	PrimaryComponentTick.bCanEverTick = false;
}

int32 UGameClockService::GetGameMinutes() const
{
	const AGameStateBase* GameState = Cast<AGameStateBase>(GetOwner());
	if (!GameState)
	{
		return 0;
	}

	return StartGameMinutes + FMath::FloorToInt32(GameState->GetServerWorldTimeSeconds() * GameMinutesPerRealSecond);
}

int32 UGameClockService::GetDay() const
{
	return GetGameMinutes() / MinutesPerDay + 1;
}

int32 UGameClockService::GetHour() const
{
	return GetGameMinutes() % MinutesPerDay / MinutesPerHour;
}

int32 UGameClockService::GetMinute() const
{
	return GetGameMinutes() % MinutesPerHour;
}

FText UGameClockService::FormatGameMinutes(int32 GameMinutes)
{
	return FText::FromString(FString::Printf(TEXT("%d일차 %02d:%02d"),
		GameMinutes / MinutesPerDay + 1, GameMinutes % MinutesPerDay / MinutesPerHour, GameMinutes % MinutesPerHour));
}
