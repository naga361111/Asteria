// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildingGridWidget.h"
#include "EngineUtils.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Building/Common/BuildingGrid.h"
#include "Building/Common/BuildingEdgeMeshData.h"

void UBuildingGridWidget::SetMeshChoices(UBuildingEdgeMeshData* InMeshData, int32 InOutlineMeshIndex, int32 InInteriorMeshIndex)
{
	MeshData = InMeshData;
	OutlineMeshIndex = InOutlineMeshIndex;
	InteriorMeshIndex = InInteriorMeshIndex;
}

void UBuildingGridWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 레벨의 첫 번째 격자.
	Grid = nullptr;
	TActorIterator<ABuildingGrid> It(GetWorld());
	if (It)
	{
		Grid = *It;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UBuildingGridWidget: no ABuildingGrid in level"));
	}

	// 탭 메뉴를 열 때마다 진행 중인 획을 초기화. 벽 변은 격자(Edges 중 Mesh 있는 변)에서 읽음.
	WallEdges.Reset();
	if (const ABuildingGrid* GridActor = Grid.Get())
	{
		for (const FBuildingGridEdge& Edge : GridActor->Edges)
		{
			if (Edge.Mesh != nullptr)
			{
				WallEdges.Add(Edge);
			}
		}
	}
	DragStart.Reset();
	HoveredPos.Reset();

	// 기본값 SelfHitTestInvisible은 마우스 이벤트를 못 받는다.
	SetVisibility(ESlateVisibility::Visible);
}

float UBuildingGridWidget::GetCellSize(const FVector2D& LocalSize) const
{
	const ABuildingGrid* GridActor = Grid.Get();
	if (!GridActor)
	{
		return 0.f;
	}

	const FIntPoint Size = GridActor->GridSize;
	return FMath::Min(LocalSize.X / Size.X, LocalSize.Y / Size.Y);
}

TOptional<FVector2D> UBuildingGridWidget::GetGridPos(const FGeometry& Geometry, const FVector2D& ScreenPos) const
{
	const float Cell = GetCellSize(Geometry.GetLocalSize());
	if (Cell <= 0.f)
	{
		return {};
	}

	return FVector2D(Geometry.AbsoluteToLocal(ScreenPos)) / Cell;
}

TArray<FBuildingGridEdge> UBuildingGridWidget::GetStrokeEdges(const FVector2D& Start, const FVector2D& End) const
{
	TArray<FBuildingGridEdge> Stroke;
	const ABuildingGrid* GridActor = Grid.Get();
	if (!GridActor)
	{
		return Stroke;
	}

	// 가장 가까운 꼭짓점으로 스냅.
	const FIntPoint Size = GridActor->GridSize;
	auto Snap = [&Size](const FVector2D& Pos)
	{
		return FIntPoint(FMath::Clamp(FMath::RoundToInt32(Pos.X), 0, Size.X), FMath::Clamp(FMath::RoundToInt32(Pos.Y), 0, Size.Y));
	};
	const FIntPoint S = Snap(Start);
	const FIntPoint E = Snap(End);

	auto AddEdge = [&Stroke](const FIntPoint& Vertex, int32 Axis)
	{
		FBuildingGridEdge& Edge = Stroke.AddDefaulted_GetRef();
		Edge.Vertex = Vertex;
		Edge.Axis = Axis;
	};

	if (S == E)
	{
		// 클릭: 가장 가까운 변 하나.
		if (const FBuildingGridEdge* Nearest = GridActor->FindNearestEdgeAt(End))
		{
			AddEdge(Nearest->Vertex, Nearest->Axis);
		}
		return Stroke;
	}

	// 드래그: 시작 꼭짓점에서 우세 축으로 끝 꼭짓점까지 직선.
	if (FMath::Abs(E.X - S.X) >= FMath::Abs(E.Y - S.Y))
	{
		for (int32 X = FMath::Min(S.X, E.X); X < FMath::Max(S.X, E.X); ++X)
		{
			AddEdge(FIntPoint(X, S.Y), 0);
		}
	}
	else
	{
		for (int32 Y = FMath::Min(S.Y, E.Y); Y < FMath::Max(S.Y, E.Y); ++Y)
		{
			AddEdge(FIntPoint(S.X, Y), 1);
		}
	}
	return Stroke;
}

UStaticMesh* UBuildingGridWidget::GetComboMesh(EBuildingEdgeKind Kind) const
{
	if (MeshData == nullptr)
	{
		return nullptr;
	}

	// 닫히지 않은 변은 아직 종류가 없으므로 외곽선 콤보.
	const bool bInterior = Kind == EBuildingEdgeKind::Interior;
	const TArray<FBuildingEdgeMeshEntry>& List = bInterior ? MeshData->InteriorMeshes : MeshData->OutlineMeshes;
	const int32 Index = bInterior ? InteriorMeshIndex : OutlineMeshIndex;
	return List.IsValidIndex(Index) ? List[Index].Mesh.Get() : nullptr;
}

const FBuildingEdgeMeshEntry* UBuildingGridWidget::FindMeshEntry(const UStaticMesh* Mesh) const
{
	if (MeshData == nullptr || Mesh == nullptr)
	{
		return nullptr;
	}

	auto IsMesh = [Mesh](const FBuildingEdgeMeshEntry& Entry)
	{
		return Entry.Mesh == Mesh;
	};
	const FBuildingEdgeMeshEntry* Entry = MeshData->OutlineMeshes.FindByPredicate(IsMesh);
	return Entry ? Entry : MeshData->InteriorMeshes.FindByPredicate(IsMesh);
}

FReply UBuildingGridWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	HoveredPos = GetGridPos(InGeometry, InMouseEvent.GetScreenSpacePosition());
	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

void UBuildingGridWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	HoveredPos.Reset();
}

FReply UBuildingGridWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Handled를 반환해야 GameAndUI 입력 모드에서 클릭이 게임 입력으로 넘어가지 않는다.
	const FKey Button = InMouseEvent.GetEffectingButton();
	if (Button != EKeys::LeftMouseButton && Button != EKeys::RightMouseButton)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	const TOptional<FVector2D> Pos = GetGridPos(InGeometry, InMouseEvent.GetScreenSpacePosition());
	if (!Pos.IsSet())
	{
		return FReply::Unhandled();
	}

	const bool bErase = Button == EKeys::RightMouseButton;
	if (!DragStart.IsSet())
	{
		// 획 시작. 왼쪽 = 그리기, 오른쪽 = 지우기. 위젯 밖에서 놓아도 받도록 캡처.
		DragStart = Pos;
		bErasing = bErase;
		return FReply::Handled().CaptureMouse(TakeWidget());
	}

	if (bErase != bErasing)
	{
		// 획 중 다른 버튼이면 취소.
		DragStart.Reset();
		return FReply::Handled().ReleaseMouseCapture();
	}
	return FReply::Handled();
}

FReply UBuildingGridWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const FKey Button = InMouseEvent.GetEffectingButton();
	const bool bErase = Button == EKeys::RightMouseButton;
	if (!DragStart.IsSet() || (Button != EKeys::LeftMouseButton && Button != EKeys::RightMouseButton) || bErase != bErasing)
	{
		return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
	}

	const TOptional<FVector2D> Pos = GetGridPos(InGeometry, InMouseEvent.GetScreenSpacePosition());
	if (!Pos.IsSet())
	{
		DragStart.Reset();
		return FReply::Handled().ReleaseMouseCapture();
	}

	const TArray<FBuildingGridEdge> Stroke = GetStrokeEdges(DragStart.GetValue(), Pos.GetValue());

	// ponytail: ABuildingGrid::SetEdgeMeshes()와 같은 선형 탐색, 변이 수백 개를 넘으면 TMap 키 조회로.
	auto FindWallEdge = [this](const FBuildingGridEdge& Key)
	{
		return WallEdges.FindByPredicate([&Key](const FBuildingGridEdge& Candidate)
		{
			return Candidate.Vertex == Key.Vertex && Candidate.Axis == Key.Axis;
		});
	};

	if (bErasing)
	{
		WallEdges.RemoveAll([&Stroke](const FBuildingGridEdge& Edge)
		{
			return Stroke.ContainsByPredicate([&Edge](const FBuildingGridEdge& Key)
			{
				return Key.Vertex == Edge.Vertex && Key.Axis == Edge.Axis;
			});
		});
	}
	else if (const ABuildingGrid* GridActor = Grid.Get())
	{
		for (const FBuildingGridEdge& Key : Stroke)
		{
			if (FindWallEdge(Key) == nullptr)
			{
				WallEdges.Add(Key);
			}
		}

		// 획의 변에 그 순간의 종류로 콤보 메시를 기록해 확정한다. 이미 있던 변이어도 덮어쓴다(변별 메시 지정).
		const TSet<FIntPoint> Floor = ABuildingGrid::GetFloorCells(WallEdges, GridActor->GridSize);
		for (const FBuildingGridEdge& Key : Stroke)
		{
			FBuildingGridEdge* Edge = FindWallEdge(Key);
			bool bUnusedFlip = false;
			if (UStaticMesh* ComboMesh = GetComboMesh(ABuildingGrid::GetEdgeKind(*Edge, Floor, bUnusedFlip)))
			{
				Edge->Mesh = ComboMesh;
			}
		}

		// 콤보 메시가 없어 기록하지 못한 새 변, 외곽선 메시를 가진 채 내부가 된 변(옆에 붙인 사각형과 맞닿은 변)은 지운다.
		// 내부 변을 지워도 바닥 칸은 그대로라 Floor를 다시 구하지 않는다.
		WallEdges.RemoveAll([this, &Floor](const FBuildingGridEdge& Edge)
		{
			if (Edge.Mesh == nullptr)
			{
				return true;
			}
			bool bUnusedFlip = false;
			return ABuildingGrid::GetEdgeKind(Edge, Floor, bUnusedFlip) == EBuildingEdgeKind::Interior
				&& MeshData != nullptr
				&& MeshData->OutlineMeshes.ContainsByPredicate([&Edge](const FBuildingEdgeMeshEntry& Entry)
				{
					return Entry.Mesh == Edge.Mesh;
				});
		});
	}

	DragStart.Reset();
	return FReply::Handled().ReleaseMouseCapture();
}

int32 UBuildingGridWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	LayerId = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	const ABuildingGrid* GridActor = Grid.Get();
	if (!GridActor)
	{
		return LayerId;
	}

	// 가로 = X(오른쪽), 세로 = Y(아래쪽). 위젯 왼쪽 위 기준.
	const FIntPoint Size = GridActor->GridSize;
	const float Cell = GetCellSize(AllottedGeometry.GetLocalSize());
	const float Width = Cell * Size.X;
	const float Height = Cell * Size.Y;

	// 칸 단위 영역(Min 포함·Max 미포함)을 격자 선 아래 레이어에 채운다.
	const FSlateBrush* Brush = FCoreStyle::Get().GetBrush("WhiteBrush");
	auto DrawRect = [&](const FIntRect& Rect, const FLinearColor& Color)
	{
		const FVector2D TopLeft(Cell * Rect.Min.X, Cell * Rect.Min.Y);
		const FVector2D BoxSize(Cell * Rect.Width(), Cell * Rect.Height());
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(BoxSize, FSlateLayoutTransform(TopLeft)), Brush, ESlateDrawEffect::None, Color);
	};

	// 벽으로 둘러싸인 바닥 칸들.
	const TSet<FIntPoint> Floor = ABuildingGrid::GetFloorCells(WallEdges, Size);
	for (const FIntPoint& FloorCell : Floor)
	{
		DrawRect(FIntRect(FloorCell, FloorCell + FIntPoint(1, 1)), FloorColor);
	}

	TArray<FVector2D> Points;
	Points.SetNum(2);

	// 가로선 GridSize.Y+1개.
	for (int32 Y = 0; Y <= Size.Y; ++Y)
	{
		Points[0] = FVector2D(0.f, Cell * Y);
		Points[1] = FVector2D(Width, Cell * Y);
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 2, AllottedGeometry.ToPaintGeometry(), Points, ESlateDrawEffect::None, LineColor);
	}

	// 세로선 GridSize.X+1개.
	for (int32 X = 0; X <= Size.X; ++X)
	{
		Points[0] = FVector2D(Cell * X, 0.f);
		Points[1] = FVector2D(Cell * X, Height);
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 2, AllottedGeometry.ToPaintGeometry(), Points, ESlateDrawEffect::None, LineColor);
	}

	// 변은 Vertex에서 Axis 방향으로 한 칸.
	auto DrawEdge = [&](const FBuildingGridEdge& Edge, const FLinearColor& Color, int32 Layer)
	{
		const FIntPoint End = Edge.Vertex + (Edge.Axis == 0 ? FIntPoint(1, 0) : FIntPoint(0, 1));
		Points[0] = FVector2D(Cell * Edge.Vertex.X, Cell * Edge.Vertex.Y);
		Points[1] = FVector2D(Cell * End.X, Cell * End.Y);
		FSlateDrawElement::MakeLines(OutDrawElements, Layer, AllottedGeometry.ToPaintGeometry(), Points, ESlateDrawEffect::None, Color, true, 3.f);
	};

	// 벽 변. 기록된 메시의 색, 목록에 없는 메시면 OpenColor. 닫히지 않은 변은 흐리게(반영 불가).
	for (const FBuildingGridEdge& Edge : WallEdges)
	{
		const FBuildingEdgeMeshEntry* Entry = FindMeshEntry(Edge.Mesh);
		FLinearColor Color = Entry ? Entry->Color : OpenColor;
		bool bUnusedFlip = false;
		if (ABuildingGrid::GetEdgeKind(Edge, Floor, bUnusedFlip) == EBuildingEdgeKind::Open)
		{
			Color.A *= 0.4f;
		}
		DrawEdge(Edge, Color, LayerId + 3);
	}

	// 획 미리보기. 획이 없으면 가리킨 변.
	if (HoveredPos.IsSet())
	{
		const FVector2D Hovered = HoveredPos.GetValue();
		const FVector2D Start = DragStart.IsSet() ? DragStart.GetValue() : Hovered;
		const FLinearColor& StrokeColor = DragStart.IsSet() && bErasing ? EraseColor : PreviewColor;
		for (const FBuildingGridEdge& Edge : GetStrokeEdges(Start, Hovered))
		{
			DrawEdge(Edge, StrokeColor, LayerId + 4);
		}
	}

	return LayerId + 4;
}

void UBuildingGridWidget::BuildWalls()
{
	ABuildingGrid* GridActor = Grid.Get();
	if (!GridActor)
	{
		return;
	}

	// 메시는 그린 시점에 확정돼 있다. 검증(Open 변·메시 null)은 ApplyWallEdges()가 한다.
	GridActor->ApplyWallEdges(WallEdges);
}
