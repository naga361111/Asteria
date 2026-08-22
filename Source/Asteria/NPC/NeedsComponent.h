// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NpcActionDef.h" // ENpcNeed
#include "NeedsComponent.generated.h"

/** 한 수치의 시간 상승 규칙 — 이 간격(게임 분)마다 이만큼 오른다. */
USTRUCT(BlueprintType)
struct FNeedRise
{
	GENERATED_BODY()

	/** 상승 간격(게임 분). AsteriaTimeSubsystem 기준. */
	UPROPERTY(EditAnywhere, Category="Needs", meta=(ClampMin="1"))
	float IntervalMinutes = 60.0f;

	/** 한 간격마다 더하는 값(0~100 clamp). */
	UPROPERTY(EditAnywhere, Category="Needs", meta=(ClampMin="0"))
	int32 PerInterval = 5;
};

/** NPC Body의 생리 수치(raw). ENpcNeed를 키로 값만 저장한다 — 종류 정의는 ENpcNeed가 단일 소스라
 *  enum에 값 추가 시 드롭다운·저장 키가 동시에 생긴다(두 곳 수정 없음).
 *  시간 증가·소비 감소 같은 계산 로직은 아직 없다. 저장 안 하는 계산 고려값은 여기 없이 enum에만 둔다. */
UCLASS(ClassGroup=(NPC), meta=(BlueprintSpawnableComponent))
class ASTERIA_API UNeedsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNeedsComponent();

	/** 고려값 종류별 raw 수치(0~100). 키 종류는 ENpcNeed에서 옴. 높을수록 결핍. 아직 아무도 안 읽음.
	 *  키는 생성자가 enum 순회로 전부 채운다 — 에디터에서는 값만 고치고 키는 추가/삭제 불가. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Needs",
		meta=(ClampMin="0", ClampMax="100", EditFixedSize, ReadOnlyKeys))
	TMap<ENpcNeed, int32> Values;

	/** 시간에 따라 오르는 수치와 그 속도. 키가 없으면 그 수치는 안 오른다.
	 *  수치 종류가 늘어도 여기에 항목만 추가하면 된다 — 코드 수정 없음. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Needs")
	TMap<ENpcNeed, FNeedRise> Rise;

	/** enum의 모든 ENpcNeed를 Values에 보장(없는 키만 0으로 추가, 기존 값 보존).
	 *  생성자 이후 enum에 값이 추가된 옛 에셋을 런타임에 메꾼다. */
	virtual void OnRegister() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	/** 수치별 다음 상승 시각(누적 게임 분). 각 수치를 처음 본 틱에 (현재 + Interval)로 잡는다. */
	TMap<ENpcNeed, double> NextRiseMinute;
};
