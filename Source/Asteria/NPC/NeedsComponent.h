// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NeedsComponent.generated.h"

/** NPC의 4대 욕구 게이지(0~100). 지금은 값만 보유한다.
 *  시간에 따른 증감(틱)·getter는 효용 루프가 이 값을 읽기 시작할 때 붙인다. */
UCLASS(ClassGroup=(NPC), meta=(BlueprintSpawnableComponent))
class ASTERIA_API UNeedsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNeedsComponent();

	/** 생존: 배고픔·갈증·수면. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Needs", meta=(ClampMin="0", ClampMax="100"))
	int32 Survival = 0;

	/** 안전: 위협(몬스터·빈곤·체력 저하)에서 벗어나려는 욕구. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Needs", meta=(ClampMin="0", ClampMax="100"))
	int32 Safety = 0;

	/** 소명: 직업적 성취·재화 축적. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Needs", meta=(ClampMin="0", ClampMax="100"))
	int32 Duty = 0;

	/** 사회: 소속감·유흥·교류. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Needs", meta=(ClampMin="0", ClampMax="100"))
	int32 Social = 0;
};
