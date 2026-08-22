// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PlaceTypes.generated.h"

/** 장소(건물) 카테고리. place 도메인 공용 어휘 — PlaceActor가 들고, Brain이 읽는다. */
UENUM(BlueprintType)
enum class EBuildingType : uint8
{
	Restaurant,
	Home,
};

/** 건물 내부 지점 종류. 각 지점 컴포넌트가 자신의 Kind로 든다. 갯수는 자유.
 *  None = 이동 아님(대기 등) 표시용 — 실제 지점에는 쓰지 않는다. */
UENUM(BlueprintType)
enum class EPlacePoint : uint8
{
	Entrance,
	Seat,
	Counter,
	None,
};
