// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/BuildingGrid.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/LineBatchComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"

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
	SpawnEdgeMeshes();
#if WITH_EDITOR
	DrawEdges();
#endif
}

void ABuildingGrid::CalculateEdges()
{
	// 다시 계산해도 편집한 Mesh가 남도록 기존 변을 따로 둔다.
	const TArray<FBuildingGridEdge> OldEdges = MoveTemp(Edges);
	Edges.Reset();

	// 가로 변(X축 방향): GridSize.X × (GridSize.Y + 1)개
	for (int32 Y = 0; Y <= GridSize.Y; ++Y)
	{
		for (int32 X = 0; X < GridSize.X; ++X)
		{
			FBuildingGridEdge& Edge = Edges.AddDefaulted_GetRef();
			Edge.Vertex = FIntPoint(X, Y);
			Edge.Axis = 0;
			Edge.Transform = FTransform(FRotator(0.f, 0.f, 0.f), FVector((X + 0.5f) * CellSize, Y * CellSize, 0.f));
		}
	}

	// 세로 변(Y축 방향): (GridSize.X + 1) × GridSize.Y개
	for (int32 X = 0; X <= GridSize.X; ++X)
	{
		for (int32 Y = 0; Y < GridSize.Y; ++Y)
		{
			FBuildingGridEdge& Edge = Edges.AddDefaulted_GetRef();
			Edge.Vertex = FIntPoint(X, Y);
			Edge.Axis = 1;
			Edge.Transform = FTransform(FRotator(0.f, 90.f, 0.f), FVector(X * CellSize, (Y + 0.5f) * CellSize, 0.f));
		}
	}

	// 같은 (Vertex, Axis) 키의 기존 변에서 Mesh를 옮겨 온다.
	// ponytail: 변마다 선형 탐색(O(n²)), 변이 수백 개를 넘으면 TMap 키 조회로.
	for (const FBuildingGridEdge& OldEdge : OldEdges)
	{
		if (OldEdge.Mesh == nullptr)
		{
			continue;
		}

		FBuildingGridEdge* NewEdge = Edges.FindByPredicate([&OldEdge](const FBuildingGridEdge& Edge)
		{
			return Edge.Vertex == OldEdge.Vertex && Edge.Axis == OldEdge.Axis;
		});
		if (NewEdge)
		{
			NewEdge->Mesh = OldEdge.Mesh;
		}
		else
		{
			// 격자 축소로 버려진 변.
			UE_LOG(LogTemp, Warning, TEXT("BuildingGrid: 변 (%d, %d) Axis %d 의 메시 %s 가 격자 축소로 버려짐"),
				OldEdge.Vertex.X, OldEdge.Vertex.Y, OldEdge.Axis, *GetNameSafe(OldEdge.Mesh));
		}
	}
}

void ABuildingGrid::SpawnEdgeMeshes()
{
	// 메시 종류마다 인스턴스드 메시 컴포넌트 하나.
	TMap<UStaticMesh*, UInstancedStaticMeshComponent*> MeshComponents;
	for (const FBuildingGridEdge& Edge : Edges)
	{
		if (Edge.Mesh == nullptr)
		{
			continue;
		}

		UInstancedStaticMeshComponent*& MeshComponent = MeshComponents.FindOrAdd(Edge.Mesh);
		if (MeshComponent == nullptr)
		{
			// 컨스트럭션 스크립트 생성으로 표시: 재실행 때 엔진이 지우고, 레벨에는 함께 저장됨.
			MeshComponent = NewObject<UInstancedStaticMeshComponent>(this);
			MeshComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
			MeshComponent->SetStaticMesh(Edge.Mesh);
			MeshComponent->SetupAttachment(RootComponent);
			MeshComponent->RegisterComponent();
		}

		// 메시 피벗이 가로·두께 중심의 바닥이므로 변 중점·회전을 그대로 쓴다.
		MeshComponent->AddInstance(Edge.Transform);
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
	for (const FBuildingGridEdge& Edge : Edges)
	{
		const FTransform& EdgeTransform = Edge.Transform;
		const FVector HalfDir = EdgeTransform.GetRotation().GetForwardVector() * (CellSize * 0.5f);
		const FVector Start = ActorTransform.TransformPosition(EdgeTransform.GetLocation() - HalfDir);
		const FVector End = ActorTransform.TransformPosition(EdgeTransform.GetLocation() + HalfDir);
		EdgeLines->DrawLine(Start, End, FLinearColor::Green, SDPG_World, 2.f);
	}
}
#endif
