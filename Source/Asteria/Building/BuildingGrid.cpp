// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/BuildingGrid.h"

#include "Components/LineBatchComponent.h"
#include "Components/SceneComponent.h"

ABuildingGrid::ABuildingGrid()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

#if WITH_EDITORONLY_DATA
	EdgeLines = CreateEditorOnlyDefaultSubobject<ULineBatchComponent>(TEXT("EdgeLines"));
	if (EdgeLines)
	{
		EdgeLines->SetupAttachment(RootComponent);
	}
#endif
}

void ABuildingGrid::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	CalculateEdges();
#if WITH_EDITOR
	DrawEdges();
#endif
}

void ABuildingGrid::CalculateEdges()
{
	Edges.Reset();

	// 가로 변(X축 방향): GridSize.X × (GridSize.Y + 1)개
	for (int32 Y = 0; Y <= GridSize.Y; ++Y)
	{
		for (int32 X = 0; X < GridSize.X; ++X)
		{
			Edges.Emplace(FRotator(0.f, 0.f, 0.f), FVector((X + 0.5f) * CellSize, Y * CellSize, 0.f));
		}
	}

	// 세로 변(Y축 방향): (GridSize.X + 1) × GridSize.Y개
	for (int32 X = 0; X <= GridSize.X; ++X)
	{
		for (int32 Y = 0; Y < GridSize.Y; ++Y)
		{
			Edges.Emplace(FRotator(0.f, 90.f, 0.f), FVector(X * CellSize, (Y + 0.5f) * CellSize, 0.f));
		}
	}
}

#if WITH_EDITOR
void ABuildingGrid::DrawEdges()
{
	if (EdgeLines == nullptr)
	{
		return;
	}

	EdgeLines->Flush();

	// 선 컴포넌트는 월드 좌표로 그리므로 액터 트랜스폼을 적용한다.
	const FTransform& ActorTransform = GetActorTransform();
	for (const FTransform& Edge : Edges)
	{
		const FVector HalfDir = Edge.GetRotation().GetForwardVector() * (CellSize * 0.5f);
		const FVector Start = ActorTransform.TransformPosition(Edge.GetLocation() - HalfDir);
		const FVector End = ActorTransform.TransformPosition(Edge.GetLocation() + HalfDir);
		EdgeLines->DrawLine(Start, End, FLinearColor::Green, SDPG_World, 2.f);
	}
}
#endif
