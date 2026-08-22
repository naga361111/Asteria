// Copyright Epic Games, Inc. All Rights Reserved.

#include "NeedsComponent.h"

#include "UObject/Class.h" // UEnum
#include "Engine/World.h"
#include "Time/AsteriaTimeSubsystem.h"

namespace
{
	/** enum의 모든 종류를 맵에 보장. 없는 키만 0으로 추가하고 기존 값은 보존한다. */
	void EnsureAllNeeds(TMap<ENpcNeed, int32>& Values)
	{
		const UEnum* NeedEnum = StaticEnum<ENpcNeed>();
		for (int32 i = 0; i < NeedEnum->NumEnums() - 1; ++i) // 마지막 자동 _MAX 항목 제외
		{
			Values.FindOrAdd(static_cast<ENpcNeed>(NeedEnum->GetValueByIndex(i)));
		}
	}
}

UNeedsComponent::UNeedsComponent()
{
	// 게임 시간 따라 허기를 올리려 틱을 켠다.
	PrimaryComponentTick.bCanEverTick = true;

	// CDO·BP 컴포넌트 템플릿에도 도니까 여기서 채워야 디테일 패널에 전 항목이 뜬다.
	EnsureAllNeeds(Values);
}

void UNeedsComponent::OnRegister()
{
	Super::OnRegister();

	// 생성자 이후 enum에 값이 추가된 옛 에셋(직렬화된 맵이 생성자를 덮어씀)을 런타임에 메꾼다.
	EnsureAllNeeds(Values);
}

void UNeedsComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const UWorld* World = GetWorld();
	const UAsteriaTimeSubsystem* Time = World ? World->GetSubsystem<UAsteriaTimeSubsystem>() : nullptr;
	if (Time == nullptr)
	{
		return;
	}

	const double Now = Time->GetTimeMinutes();

	for (const TPair<ENpcNeed, FNeedRise>& Entry : Rise)
	{
		const float Interval = Entry.Value.IntervalMinutes;
		if (Interval <= 0.0f)
		{
			continue; // 0이면 while이 무한 루프 — 안 오르는 것으로 취급.
		}

		double* Next = NextRiseMinute.Find(Entry.Key);
		if (Next == nullptr)
		{
			// 이 수치를 처음 본 틱: 시작 즉시 안 오르게 다음 시각만 잡고 끝.
			NextRiseMinute.Add(Entry.Key, Now + Interval);
			continue;
		}

		// 한 프레임에 여러 간격이 지나도(긴 델타) 빠짐없이 처리. 보통 1회.
		int32& Value = Values.FindOrAdd(Entry.Key);
		while (Now >= *Next)
		{
			Value = FMath::Clamp(Value + Entry.Value.PerInterval, 0, 100);
			*Next += Interval;
		}
	}
}
