// Copyright Epic Games, Inc. All Rights Reserved.

#include "AsteriaTimeSubsystem.h"

#include "Engine/Engine.h"

namespace
{
	constexpr double MinutesPerHour = 60.0;
	constexpr double MinutesPerDay = 1440.0;
	constexpr uint64 GTimeDebugKey = 0; // 온스크린 메시지 고정 슬롯(작은 키 = 상단).
}

void UAsteriaTimeSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 멈추지 않고 흐른다. pause 때는 틱 자체가 안 오므로 그대로 멈춤(현행 유지).
	CurrentMinutes += DeltaTime * Scale;

#if !UE_BUILD_SHIPPING
	// 디버그용 상시 표시 — 화면 상단에 Day/Hour 고정 갱신(같은 키라 스팸 없이 값만 바뀜).
	if (GEngine != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(GTimeDebugKey, /*TimeToDisplay=*/2.0f, FColor::White,
			FString::Printf(TEXT("Day %d  %02d:%02d"), GetDay(), GetHour(), GetMinute()));
	}
#endif
}

int32 UAsteriaTimeSubsystem::GetDay() const
{
	return FMath::FloorToInt(CurrentMinutes / MinutesPerDay);
}

int32 UAsteriaTimeSubsystem::GetHour() const
{
	return FMath::FloorToInt(FMath::Fmod(CurrentMinutes, MinutesPerDay) / MinutesPerHour);
}

int32 UAsteriaTimeSubsystem::GetMinute() const
{
	return FMath::FloorToInt(FMath::Fmod(CurrentMinutes, MinutesPerHour));
}

TStatId UAsteriaTimeSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAsteriaTimeSubsystem, STATGROUP_Tickables);
}
