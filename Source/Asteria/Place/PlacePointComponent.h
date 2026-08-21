// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "PlaceTypes.h"
#include "PlacePointComponent.generated.h"

/** 건물 내부의 한 목적지 지점. 건물 BP 뷰포트에 놓아 위치를 잡고, Kind로 종류를 표시한다.
 *  같은 액터의 UPlaceComponent가 Kind로 걷어간다. 같은 Kind를 원하는 만큼 놓아도 됨. */
UCLASS(ClassGroup=(Place), meta=(BlueprintSpawnableComponent))
class ASTERIA_API UPlacePointComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	/** 이 지점의 종류(입구/좌석 등). */
	UPROPERTY(EditAnywhere, Category="Place")
	EPlacePoint Kind = EPlacePoint::Seat;
};
