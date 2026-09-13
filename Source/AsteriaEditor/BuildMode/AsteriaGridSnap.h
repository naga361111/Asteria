// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 그리드 스냅 계산. 배치 툴과 고스트 프리뷰가 반드시 같은 값을 써야 하므로 어느 쪽 클래스에도
 * 넣지 않고 여기 한 곳에만 둔다. 소켓 스냅이 들어올 때 갈아끼울 이음매도 여기다.
 */
namespace AsteriaBuild
{
	/**
	 * XY는 커서가 가리킨 지점을 격자에 맞추고, Z는 호출자가 정한 층 높이를 그대로 쓴다.
	 *
	 * Z를 인자로 받는 게 핵심이다. Z를 지오메트리 좌표에서 뽑아 반올림하면, 그 좌표가 반올림
	 * 경계에 앉는 순간(피벗이 가운데인 피스가 딱 그렇다) 같은 자리에서도 층이 뒤집힌다.
	 * Z는 연속 좌표가 아니라 이산 상태에서 와야 한다.
	 */
	inline FTransform SnapToGrid(const FVector& SurfacePoint, double PlacementZ, float GridSize, float Yaw)
	{
		const FVector Snapped(
			FMath::GridSnap(SurfacePoint.X, static_cast<double>(GridSize)),
			FMath::GridSnap(SurfacePoint.Y, static_cast<double>(GridSize)),
			PlacementZ);

		return FTransform(FRotator(0.0, Yaw, 0.0), Snapped);
	}
}
