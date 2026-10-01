// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildingGrid.generated.h"

class ULineBatchComponent;
class UStaticMesh;

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

	virtual void OnConstruction(const FTransform& Transform) override;

	// 광선이 격자 바닥 평면과 만나는 점에서 가장 가까운 변. 평면과 안 만나거나 격자 밖이면 nullptr.
	const FBuildingGridEdge* FindNearestEdge(const FRay& Ray) const;

	// (Vertex, Axis) 키의 변에 Mesh를 할당하고 인스턴스 메시를 다시 만든다. 키에 해당하는 변이 없으면 무시.
	void SetEdgeMesh(const FIntPoint& Vertex, int32 Axis, UStaticMesh* Mesh);

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
