// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlaceTypes.h"
#include "PlaceComponent.generated.h"

/** 이 액터가 무슨 건물인지 표시하는 카테고리 태그. 아무 액터 BP에 붙여 쓴다.
 *  Brain이 이 Type을 읽어 행동 목적지 건물을 고른다. */
UCLASS(ClassGroup=(Place), meta=(BlueprintSpawnableComponent))
class ASTERIA_API UPlaceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** 이 건물의 카테고리. BP/인스턴스에서 지정. */
	UPROPERTY(EditAnywhere, Category="Place")
	EBuildingType Type = EBuildingType::Restaurant;

	/** 소유 액터에 놓인 UPlacePointComponent 중 Kind가 일치하는 지점들의 월드 위치.
	 *  저장 배열 없음 — 매 호출 시 소유 액터 컴포넌트를 훑는다(건물당 점 수는 소규모). */
	TArray<FVector> GetPoints(EPlacePoint Kind) const;
};
