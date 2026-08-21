// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TemperamentComponent.generated.h"

class UDataTable;

/** NPC의 기질(4기둥 중 하나). 기질 테이블 하나를 가리키고, 그 안의 행(기질 종류)을 여러 개 고른다.
 *  배율을 실제로 곱하는 계산은 효용 루프가 이 값을 읽기 시작할 때 붙인다. */
UCLASS(ClassGroup=(NPC), meta=(BlueprintSpawnableComponent))
class ASTERIA_API UTemperamentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTemperamentComponent();

	/** 기질 테이블. 행 하나 = 기질 종류 하나(FNpcTemperamentDef). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Temperament")
	TObjectPtr<UDataTable> Table;

	/** 이 NPC가 가진 기질들 = Table의 행 이름 집합. TSet이라 같은 종류 두 번 못 들어감. 비면 평범.
	 *  GetOptions로 Table의 행 이름만 체크박스로 뜬다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Temperament", meta=(GetOptions="GetRowNames"))
	TSet<FName> Temperaments;

private:
	/** GetOptions용 — 현재 Table의 행 이름 목록. */
	UFUNCTION()
	TArray<FName> GetRowNames() const;
};
