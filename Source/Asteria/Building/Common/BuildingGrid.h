// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildingGrid.generated.h"

class ULineBatchComponent;
class UStaticMesh;

// 바닥 칸 기준 변 종류. Open = 양쪽 모두 바닥 아님(닫히지 않은 변), Outline = 한쪽만 바닥, Interior = 양쪽 모두 바닥.
enum class EBuildingEdgeKind : uint8
{
	Open,
	Outline,
	Interior,
};

// 격자 변 하나의 데이터. 변마다 데이터를 붙이는 단위.
USTRUCT()
struct FBuildingGridEdge
{
	GENERATED_BODY()

	// 변의 시작 꼭짓점 격자 좌표. Axis와 함께 변 키.
	UPROPERTY(VisibleAnywhere, Category = "Grid")
	FIntPoint Vertex = FIntPoint::ZeroValue;

	// 변 방향. 0 = X축, 1 = Y축. Vertex와 함께 변 키.
	UPROPERTY(VisibleAnywhere, Category = "Grid")
	int32 Axis = 0;

	// 액터 기준. 위치 = 변 중점, yaw 0 = X축 방향, 90 = Y축 방향.
	UPROPERTY(VisibleAnywhere, Category = "Grid")
	FTransform Transform;

	// 이 변에 놓을 메시. null = 빈 변.
	UPROPERTY(EditAnywhere, Category = "Grid")
	TObjectPtr<UStaticMesh> Mesh;

	// true면 이 변의 메시를 yaw 180도로 놓는다.
	UPROPERTY(EditAnywhere, Category = "Grid")
	bool bFlip = false;
};

// 레벨에 배치하는 격자. 액터 트랜스폼이 격자 원점이고, 격자의 각 변 위치를 계산한다.
UCLASS()
class ASTERIA_API ABuildingGrid : public AActor
{
	GENERATED_BODY()

public:
	ABuildingGrid();

	// 한 칸(변 길이).
	static constexpr float CellSize = 300.f;

	// 가로(X)·세로(Y) 칸 수.
	UPROPERTY(EditAnywhere, Category = "Grid", meta = (ClampMin = "1"))
	FIntPoint GridSize = FIntPoint(3, 2);

	// 격자의 모든 변. CalculateEdges()가 채움. 원소 추가·삭제 불가, Mesh만 편집.
	UPROPERTY(EditAnywhere, EditFixedSize, Category = "Grid")
	TArray<FBuildingGridEdge> Edges;

	// 건물 바닥 칸 집합. 벽 변으로 둘러싸인 칸. ApplyWallEdges()가 설정.
	UPROPERTY(VisibleAnywhere, Category = "Grid")
	TSet<FIntPoint> FloorCells;

	virtual void OnConstruction(const FTransform& Transform) override;

	// 광선이 격자 바닥 평면과 만나는 점에서 가장 가까운 변. 평면과 안 만나거나 격자 밖이면 nullptr.
	const FBuildingGridEdge* FindNearestEdge(const FRay& Ray) const;

	// 격자 칸 단위 좌표(액터 기준 X/CellSize, Y/CellSize)에서 가장 가까운 변. 후보가 없으면 nullptr.
	const FBuildingGridEdge* FindNearestEdgeAt(const FVector2D& GridPos) const;

	// NewEdges 각 원소의 (Vertex, Axis) 키 변에 그 원소의 Mesh·bFlip을 넣고 인스턴스 메시를 한 번만 다시 만든다.
	// 원소의 Transform은 무시. 키에 해당하는 변이 없는 원소는 무시.
	void SetEdgeMeshes(const TArray<FBuildingGridEdge>& NewEdges);

	// Edge에 메시를 놓을 액터 기준 트랜스폼. bFlip이면 변 Transform에 yaw 180도를 더한다(위치는 그대로).
	static FTransform GetEdgeMeshTransform(const FBuildingGridEdge& Edge, bool bFlip);

	// WallEdges(Vertex·Axis만 사용)로 둘러싸인 칸 집합.
	static TSet<FIntPoint> GetFloorCells(const TArray<FBuildingGridEdge>& WallEdges, const FIntPoint& InGridSize);

	// Edge 양옆 칸의 바닥 여부로 종류. bOutInwardFlip = 뒤집지 않은 메시의 앞쪽 칸이 바닥이 아니면 true(Outline일 때 안쪽을 향하게 하는 bFlip).
	static EBuildingEdgeKind GetEdgeKind(const FBuildingGridEdge& Edge, const TSet<FIntPoint>& InFloorCells, bool& bOutInwardFlip);

	// NewWallEdges를 격자 변 메시로 반영하고 FloorCells를 갱신. Open이거나 Mesh가 null인 변이 있으면 false·변경 없음.
	bool ApplyWallEdges(const TArray<FBuildingGridEdge>& NewWallEdges);

private:
	void CalculateEdges();
	void SpawnEdgeMeshes();

#if WITH_EDITORONLY_DATA
	// 변을 그리는 에디터 전용 선. DrawEdges()가 채움.
	UPROPERTY(Transient)
	TObjectPtr<ULineBatchComponent> EdgeLines;
#endif

#if WITH_EDITOR
	void DrawEdges();
#endif
};
