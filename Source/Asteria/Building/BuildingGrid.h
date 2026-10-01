// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildingGrid.generated.h"

class ULineBatchComponent;

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

	// 격자의 모든 변(액터 기준). 위치 = 변 중점, yaw 0 = X축 방향, 90 = Y축 방향. CalculateEdges()가 채움.
	UPROPERTY(VisibleAnywhere, Category = "Grid")
	TArray<FTransform> Edges;

	virtual void OnConstruction(const FTransform& Transform) override;

private:
	void CalculateEdges();

#if WITH_EDITORONLY_DATA
	// 변을 그리는 에디터 전용 선. DrawEdges()가 채움.
	UPROPERTY(Transient)
	TObjectPtr<ULineBatchComponent> EdgeLines;
#endif

#if WITH_EDITOR
	void DrawEdges();
#endif
};
