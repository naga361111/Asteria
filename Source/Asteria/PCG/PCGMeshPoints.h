#pragma once

#include "CoreMinimal.h"

struct FPCGContext;
class UPCGBasePointData;

// 메시 경로를 담은 점 데이터를 내는 PCG 노드(건물·카운터) 공용 헬퍼.
namespace PCGMeshPoints
{
	// 점의 메시 경로 속성. Static Mesh Spawner(By Attribute)가 읽는다.
	extern const FName MeshAttribute;

	// (0,0,0) = 볼륨 로컬 경계 최소 모서리(bBottomCenter면 바닥면 가운데). 볼륨 위치·회전은 따르고 스케일은 빼 메시 크기를 유지한다.
	// 실행 소스가 없으면 항등.
	FTransform GetVolumeOrigin(const FPCGContext* Context, bool bBottomCenter = false);

	// 점마다 Mesh 속성을 단 점 데이터.
	UPCGBasePointData* MakePointData(FPCGContext* Context, const TArray<FTransform>& Transforms, const TArray<FSoftObjectPath>& Meshes);
}
