// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "NpcDebugSettings.generated.h"

/** Project Settings > Asteria > NPC Debug. NPC 디버그 트레이스/그리기 수치. */
UCLASS(config=Game, defaultconfig, meta=(DisplayName="NPC Debug"))
class ASTERIA_API UNpcDebugSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return FName(TEXT("Asteria")); }

	/** look-at 트레이스 사거리(cm). */
	UPROPERTY(EditAnywhere, config, Category="Look At", meta=(ClampMin="0"))
	float TraceRange = 5000.0f;

	/** 디버그 라인 그리기 시작점 오프셋. X=카메라 우측, Y=카메라 상하(view-local, cm).
	 *  트레이스 판정 시작점(화면 정중앙)에는 영향 없음. */
	UPROPERTY(EditAnywhere, config, Category="Look At")
	FVector2D DrawOffset = FVector2D(0.0f, -20.0f);

	/** 디버그 키(-/=)로 욕구를 한 번에 얼마나 증감할지. */
	UPROPERTY(EditAnywhere, config, Category="Needs Edit", meta=(ClampMin="1"))
	int32 NeedStep = 10;
};
