// Fill out your copyright notice in the Description page of Project Settings.


#include "Building/Common/BuildingGrid.h"

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

	// 같은 (Vertex, Axis) 키의 기존 변에서 Mesh와 bFlip을 옮겨 온다.
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
			NewEdge->bFlip = OldEdge.bFlip;
		}
		else
		{
			// 격자 축소로 버려진 변.
			UE_LOG(LogTemp, Warning, TEXT("BuildingGrid: 변 (%d, %d) Axis %d 의 메시 %s 가 격자 축소로 버려짐"),
				OldEdge.Vertex.X, OldEdge.Vertex.Y, OldEdge.Axis, *GetNameSafe(OldEdge.Mesh));
		}
	}
}

const FBuildingGridEdge* ABuildingGrid::FindNearestEdge(const FRay& Ray) const
{
	// 바닥 평면: 액터 위치를 지나고 액터 위쪽에 수직.
	const FVector PlaneNormal = GetActorUpVector();
	const double Denom = FVector::DotProduct(Ray.Direction, PlaneNormal);
	if (FMath::IsNearlyZero(Denom))
	{
		return nullptr;
	}
	const double T = FVector::DotProduct(GetActorLocation() - Ray.Origin, PlaneNormal) / Denom;
	if (T < 0.0)
	{
		return nullptr;
	}

	// 만난 점을 액터 기준 칸 좌표로.
	const FVector Local = GetActorTransform().InverseTransformPosition(Ray.PointAt(T));
	const double U = Local.X / CellSize;
	const double V = Local.Y / CellSize;

	// ponytail: CalculateEdges()와 같은 선형 탐색, 변이 수백 개를 넘으면 TMap 키 조회로.
	auto FindEdge = [this](const FIntPoint& Vertex, int32 Axis)
	{
		return Edges.FindByPredicate([&Vertex, Axis](const FBuildingGridEdge& Edge)
		{
			return Edge.Vertex == Vertex && Edge.Axis == Axis;
		});
	};

	// 가로 변 후보: 가장 가까운 가로 선 위의 칸. 세로 변도 마찬가지.
	const int32 RoundU = FMath::RoundToInt32(U);
	const int32 RoundV = FMath::RoundToInt32(V);
	const FBuildingGridEdge* XEdge = FindEdge(FIntPoint(FMath::FloorToInt32(U), RoundV), 0);
	const FBuildingGridEdge* YEdge = FindEdge(FIntPoint(RoundU, FMath::FloorToInt32(V)), 1);
	if (XEdge == nullptr || YEdge == nullptr)
	{
		return XEdge ? XEdge : YEdge;
	}
	return FMath::Abs(V - RoundV) <= FMath::Abs(U - RoundU) ? XEdge : YEdge;
}

void ABuildingGrid::SetEdgeMeshes(const TArray<FBuildingGridEdge>& NewEdges)
{
	for (const FBuildingGridEdge& NewEdge : NewEdges)
	{
		// ponytail: CalculateEdges()와 같은 선형 탐색, 변이 수백 개를 넘으면 TMap 키 조회로.
		FBuildingGridEdge* Edge = Edges.FindByPredicate([&NewEdge](const FBuildingGridEdge& Candidate)
		{
			return Candidate.Vertex == NewEdge.Vertex && Candidate.Axis == NewEdge.Axis;
		});
		if (Edge == nullptr)
		{
			continue;
		}
		Edge->Mesh = NewEdge.Mesh;
		Edge->bFlip = NewEdge.bFlip;
	}

	// 이 격자의 인스턴스 메시 컴포넌트는 SpawnEdgeMeshes()가 만든 것뿐. 실행 중에는 엔진이 지워 주지 않으므로 직접 파괴.
	TArray<UInstancedStaticMeshComponent*> MeshComponents;
	GetComponents(MeshComponents);
	for (UInstancedStaticMeshComponent* MeshComponent : MeshComponents)
	{
		MeshComponent->DestroyComponent();
	}

	SpawnEdgeMeshes();
}

FTransform ABuildingGrid::GetEdgeMeshTransform(const FBuildingGridEdge& Edge, bool bFlip)
{
	// 메시 피벗이 변 중점의 바닥이므로 회전만 더하면 같은 변에 반대 방향으로 놓인다.
	FTransform Result = Edge.Transform;
	if (bFlip)
	{
		Result.ConcatenateRotation(FRotator(0.f, 180.f, 0.f).Quaternion());
	}
	return Result;
}

TArray<FBuildingGridEdge> ABuildingGrid::GetOutlineEdges(const TSet<FIntPoint>& Cells, const FIntPoint& InGridSize)
{
	TArray<FBuildingGridEdge> Result;
	auto AddEdge = [&Result](int32 X, int32 Y, int32 Axis, bool bFlip)
	{
		FBuildingGridEdge& Edge = Result.AddDefaulted_GetRef();
		Edge.Vertex = FIntPoint(X, Y);
		Edge.Axis = Axis;
		Edge.bFlip = bFlip;
	};

	// 외곽선: 양쪽 칸 중 하나만 Cells에 있는 변. Vertex = 변의 시작 꼭짓점. 격자 밖 칸은 Cells 밖.
	// 뒤집지 않은 메시의 앞은 로컬 -Y(AAsteriaPlayer::ShouldFlip). 앞쪽 칸이 Cells 밖이면 뒤집어 안쪽을 향하게 한다.
	// 가로 변(Axis 0): 칸 (X, Y-1)과 (X, Y) 사이. 앞(-Y)은 (X, Y-1) 쪽.
	for (int32 Y = 0; Y <= InGridSize.Y; ++Y)
	{
		for (int32 X = 0; X < InGridSize.X; ++X)
		{
			const bool bFront = Cells.Contains(FIntPoint(X, Y - 1));
			if (bFront != Cells.Contains(FIntPoint(X, Y)))
			{
				AddEdge(X, Y, 0, !bFront);
			}
		}
	}

	// 세로 변(Axis 1): 칸 (X-1, Y)와 (X, Y) 사이. yaw 90이라 앞은 +X, 칸 (X, Y) 쪽.
	for (int32 X = 0; X <= InGridSize.X; ++X)
	{
		for (int32 Y = 0; Y < InGridSize.Y; ++Y)
		{
			const bool bFront = Cells.Contains(FIntPoint(X, Y));
			if (Cells.Contains(FIntPoint(X - 1, Y)) != bFront)
			{
				AddEdge(X, Y, 1, !bFront);
			}
		}
	}

	return Result;
}

void ABuildingGrid::ApplyFloorCells(const TSet<FIntPoint>& NewCells, UStaticMesh* WallMesh)
{
	if (WallMesh == nullptr)
	{
		return;
	}

	// 이전·새 외곽선 어디에도 없는 변은 넘기지 않으므로 기존 메시가 그대로 남는다. 로컬 호출만.
	const TArray<FBuildingGridEdge> OldOutline = GetOutlineEdges(FloorCells, GridSize);
	TArray<FBuildingGridEdge> NewEdges = GetOutlineEdges(NewCells, GridSize);
	for (FBuildingGridEdge& Edge : NewEdges)
	{
		Edge.Mesh = WallMesh;
	}

	// 이전 외곽선에만 있던 변은 비운다. 뒤에 붙인 원소도 이전 외곽선끼리 키가 겹치지 않으므로 탐색 대상에 섞여도 무방.
	for (const FBuildingGridEdge& OldEdge : OldOutline)
	{
		// ponytail: CalculateEdges()와 같은 선형 탐색, 변이 수백 개를 넘으면 TMap 키 조회로.
		const bool bStillOutline = NewEdges.ContainsByPredicate([&OldEdge](const FBuildingGridEdge& Candidate)
		{
			return Candidate.Vertex == OldEdge.Vertex && Candidate.Axis == OldEdge.Axis;
		});
		if (!bStillOutline)
		{
			FBuildingGridEdge& Edge = NewEdges.Add_GetRef(OldEdge);
			Edge.Mesh = nullptr;
		}
	}

	FloorCells = NewCells;
	if (!NewEdges.IsEmpty())
	{
		SetEdgeMeshes(NewEdges);
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

		MeshComponent->AddInstance(GetEdgeMeshTransform(Edge, Edge.bFlip));
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
