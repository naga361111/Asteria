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
	return FindNearestEdgeAt(FVector2D(Local.X / CellSize, Local.Y / CellSize));
}

const FBuildingGridEdge* ABuildingGrid::FindNearestEdgeAt(const FVector2D& GridPos) const
{
	const double U = GridPos.X;
	const double V = GridPos.Y;

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

TSet<FIntPoint> ABuildingGrid::GetFloorCells(const TArray<FBuildingGridEdge>& WallEdges, const FIntPoint& InGridSize)
{
	// ponytail: 벽으로 둘러싸인 빈 공간(안뜰)도 바닥으로 판정, 구분이 필요하면 바닥 칸을 따로 지정.
	TSet<FIntVector> Walls;
	for (const FBuildingGridEdge& Edge : WallEdges)
	{
		Walls.Add(FIntVector(Edge.Vertex.X, Edge.Vertex.Y, Edge.Axis));
	}

	// 격자 밖 한 칸까지 포함한 범위에서 (-1, -1)부터 벽을 건너지 않고 닿는 칸을 찾는다.
	auto InRange = [&InGridSize](const FIntPoint& Cell)
	{
		return Cell.X >= -1 && Cell.X <= InGridSize.X && Cell.Y >= -1 && Cell.Y <= InGridSize.Y;
	};

	TSet<FIntPoint> Visited;
	TArray<FIntPoint> Queue;
	Visited.Add(FIntPoint(-1, -1));
	Queue.Add(FIntPoint(-1, -1));
	for (int32 Head = 0; Head < Queue.Num(); ++Head)
	{
		const FIntPoint Cell = Queue[Head];
		auto TryVisit = [&](const FIntPoint& Next, const FIntVector& Wall)
		{
			if (InRange(Next) && !Walls.Contains(Wall) && !Visited.Contains(Next))
			{
				Visited.Add(Next);
				Queue.Add(Next);
			}
		};

		// 칸 (X, Y)↔(X+1, Y) 사이 = Axis 1, Vertex (X+1, Y). 칸 (X, Y)↔(X, Y+1) 사이 = Axis 0, Vertex (X, Y+1).
		TryVisit(FIntPoint(Cell.X + 1, Cell.Y), FIntVector(Cell.X + 1, Cell.Y, 1));
		TryVisit(FIntPoint(Cell.X - 1, Cell.Y), FIntVector(Cell.X, Cell.Y, 1));
		TryVisit(FIntPoint(Cell.X, Cell.Y + 1), FIntVector(Cell.X, Cell.Y + 1, 0));
		TryVisit(FIntPoint(Cell.X, Cell.Y - 1), FIntVector(Cell.X, Cell.Y, 0));
	}

	TSet<FIntPoint> Result;
	for (int32 Y = 0; Y < InGridSize.Y; ++Y)
	{
		for (int32 X = 0; X < InGridSize.X; ++X)
		{
			if (!Visited.Contains(FIntPoint(X, Y)))
			{
				Result.Add(FIntPoint(X, Y));
			}
		}
	}
	return Result;
}

EBuildingEdgeKind ABuildingGrid::GetEdgeKind(const FBuildingGridEdge& Edge, const TSet<FIntPoint>& InFloorCells, bool& bOutInwardFlip)
{
	// 뒤집지 않은 메시의 앞은 로컬 -Y(AAsteriaPlayer::ShouldFlip). 앞쪽 칸이 바닥 밖이면 뒤집어 안쪽을 향하게 한다.
	// 가로 변(Axis 0): 칸 (X, Y-1)과 (X, Y) 사이. 앞(-Y)은 (X, Y-1) 쪽.
	// 세로 변(Axis 1): 칸 (X-1, Y)와 (X, Y) 사이. yaw 90이라 앞은 +X, 칸 (X, Y) 쪽.
	const FIntPoint& V = Edge.Vertex;
	const FIntPoint Front = Edge.Axis == 0 ? FIntPoint(V.X, V.Y - 1) : FIntPoint(V.X, V.Y);
	const FIntPoint Back = Edge.Axis == 0 ? FIntPoint(V.X, V.Y) : FIntPoint(V.X - 1, V.Y);

	const bool bFront = InFloorCells.Contains(Front);
	const bool bBack = InFloorCells.Contains(Back);
	bOutInwardFlip = !bFront;
	if (bFront && bBack)
	{
		return EBuildingEdgeKind::Interior;
	}
	return bFront || bBack ? EBuildingEdgeKind::Outline : EBuildingEdgeKind::Open;
}

bool ABuildingGrid::ApplyWallEdges(const TArray<FBuildingGridEdge>& NewWallEdges)
{
	// 검증을 반영 전에 전부 끝낸다. 일부만 반영되는 경우 없음. 로컬 호출만(서버 권위는 추후).
	const TSet<FIntPoint> NewFloor = GetFloorCells(NewWallEdges, GridSize);
	TArray<FBuildingGridEdge> NewEdges = NewWallEdges;
	for (FBuildingGridEdge& Edge : NewEdges)
	{
		bool bInward = false;
		const EBuildingEdgeKind Kind = GetEdgeKind(Edge, NewFloor, bInward);
		if (Kind == EBuildingEdgeKind::Open || Edge.Mesh == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("BuildingGrid: 변 (%d, %d) Axis %d 가 닫히지 않았거나 메시가 없어 벽 반영 취소"),
				Edge.Vertex.X, Edge.Vertex.Y, Edge.Axis);
			return false;
		}
		if (Kind == EBuildingEdgeKind::Outline)
		{
			Edge.bFlip = bInward;
		}
	}

	// 이전에 메시가 있었지만 새 벽에서 빠진 변은 비운다.
	for (const FBuildingGridEdge& OldEdge : Edges)
	{
		if (OldEdge.Mesh == nullptr)
		{
			continue;
		}

		// ponytail: CalculateEdges()와 같은 선형 탐색, 변이 수백 개를 넘으면 TMap 키 조회로.
		const bool bStillWall = NewWallEdges.ContainsByPredicate([&OldEdge](const FBuildingGridEdge& Candidate)
		{
			return Candidate.Vertex == OldEdge.Vertex && Candidate.Axis == OldEdge.Axis;
		});
		if (!bStillWall)
		{
			FBuildingGridEdge& Edge = NewEdges.Add_GetRef(OldEdge);
			Edge.Mesh = nullptr;
		}
	}

	FloorCells = NewFloor;
	if (!NewEdges.IsEmpty())
	{
		SetEdgeMeshes(NewEdges);
	}
	return true;
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
