// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildingGridWidget.h"
#include "EngineUtils.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Building/Common/BuildingGrid.h"

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

	// 탭 메뉴를 열 때마다 선택 초기화.
	SelectionAnchor.Reset();
	HoveredCell.Reset();
	SelectedRects.Reset();

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

TOptional<FIntPoint> UBuildingGridWidget::GetCellAt(const FGeometry& Geometry, const FVector2D& ScreenPos) const
{
	const float Cell = GetCellSize(Geometry.GetLocalSize());
	if (Cell <= 0.f)
	{
		return {};
	}

	const FVector2D Local = Geometry.AbsoluteToLocal(ScreenPos);
	const FIntPoint CellPos(FMath::FloorToInt32(Local.X / Cell), FMath::FloorToInt32(Local.Y / Cell));
	const FIntPoint Size = Grid->GridSize;
	if (CellPos.X < 0 || CellPos.X >= Size.X || CellPos.Y < 0 || CellPos.Y >= Size.Y)
	{
		return {};
	}
	return CellPos;
}

bool UBuildingGridWidget::OverlapsSelectedRects(const FIntRect& Rect) const
{
	// Intersect는 반열린 사각형 기준이라 변만 맞닿으면 false.
	for (const FIntRect& Selected : SelectedRects)
	{
		if (Selected.Intersect(Rect))
		{
			return true;
		}
	}
	return false;
}

bool UBuildingGridWidget::IsCellSelected(const FIntPoint& Cell) const
{
	// Contains는 Min 포함·Max 미포함.
	for (const FIntRect& Selected : SelectedRects)
	{
		if (Selected.Contains(Cell))
		{
			return true;
		}
	}
	return false;
}

FReply UBuildingGridWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	HoveredCell = GetCellAt(InGeometry, InMouseEvent.GetScreenSpacePosition());
	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

void UBuildingGridWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	HoveredCell.Reset();
}

FReply UBuildingGridWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Handled를 반환해야 GameAndUI 입력 모드에서 클릭이 게임 입력으로 넘어가지 않는다.
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		const TOptional<FIntPoint> CellPos = GetCellAt(InGeometry, InMouseEvent.GetScreenSpacePosition());
		if (!CellPos.IsSet())
		{
			return FReply::Unhandled();
		}

		if (!SelectionAnchor.IsSet())
		{
			// 첫 모서리.
			SelectionAnchor = CellPos;
		}
		else
		{
			// 반대 모서리로 확정. 기존 사각형과 겹치면 무시하고 첫 모서리 유지.
			const FIntPoint A = SelectionAnchor.GetValue();
			const FIntPoint B = CellPos.GetValue();
			const FIntRect Rect(A.ComponentMin(B), A.ComponentMax(B) + FIntPoint(1, 1));
			if (OverlapsSelectedRects(Rect))
			{
				return FReply::Handled();
			}
			SelectedRects.Add(Rect);
			SelectionAnchor.Reset();
		}
		return FReply::Handled();
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		// 선택 취소.
		SelectionAnchor.Reset();
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
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

	FLinearColor PreviewColor = SelectionColor;
	PreviewColor.A *= 0.5f;

	// 확정 사각형들.
	for (const FIntRect& Selected : SelectedRects)
	{
		DrawRect(Selected, SelectionColor);
	}

	if (HoveredCell.IsSet())
	{
		const FIntPoint H = HoveredCell.GetValue();
		if (SelectionAnchor.IsSet())
		{
			// 미리보기 사각형. 기존 사각형과 겹치면 빨강.
			const FIntPoint A = SelectionAnchor.GetValue();
			const FIntRect Preview(A.ComponentMin(H), A.ComponentMax(H) + FIntPoint(1, 1));
			DrawRect(Preview, OverlapsSelectedRects(Preview) ? FLinearColor(1.f, 0.f, 0.f, PreviewColor.A) : PreviewColor);
		}
		else
		{
			// 가리킨 칸.
			DrawRect(FIntRect(H, H + FIntPoint(1, 1)), PreviewColor);
		}
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

	// 외곽선. 변은 Vertex에서 Axis 방향으로 한 칸.
	for (const FBuildingGridEdge& Edge : GetOutlineEdges())
	{
		const FIntPoint End = Edge.Vertex + (Edge.Axis == 0 ? FIntPoint(1, 0) : FIntPoint(0, 1));
		Points[0] = FVector2D(Cell * Edge.Vertex.X, Cell * Edge.Vertex.Y);
		Points[1] = FVector2D(Cell * End.X, Cell * End.Y);
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 3, AllottedGeometry.ToPaintGeometry(), Points, ESlateDrawEffect::None, OutlineColor, true, 3.f);
	}

	return LayerId + 3;
}

TArray<FBuildingGridEdge> UBuildingGridWidget::GetOutlineEdges() const
{
	TArray<FBuildingGridEdge> Result;
	const ABuildingGrid* GridActor = Grid.Get();
	if (!GridActor)
	{
		return Result;
	}

	const FIntPoint Size = GridActor->GridSize;
	auto AddEdge = [&Result](int32 X, int32 Y, int32 Axis, bool bFlip)
	{
		FBuildingGridEdge& Edge = Result.AddDefaulted_GetRef();
		Edge.Vertex = FIntPoint(X, Y);
		Edge.Axis = Axis;
		Edge.bFlip = bFlip;
	};

	// 외곽선: 양쪽 칸 중 하나만 선택된 변. 변 정의는 ABuildingGrid와 같음(Vertex = 변의 시작 꼭짓점). 격자 밖 칸은 미선택.
	// 뒤집지 않은 메시의 앞은 로컬 -Y(AAsteriaPlayer::ShouldFlip). 앞쪽 칸이 선택 영역 밖이면 뒤집어 안쪽을 향하게 한다.
	// 가로 변(Axis 0): 칸 (X, Y-1)과 (X, Y) 사이. 앞(-Y)은 (X, Y-1) 쪽.
	for (int32 Y = 0; Y <= Size.Y; ++Y)
	{
		for (int32 X = 0; X < Size.X; ++X)
		{
			const bool bFront = IsCellSelected(FIntPoint(X, Y - 1));
			if (bFront != IsCellSelected(FIntPoint(X, Y)))
			{
				AddEdge(X, Y, 0, !bFront);
			}
		}
	}

	// 세로 변(Axis 1): 칸 (X-1, Y)와 (X, Y) 사이. yaw 90이라 앞은 +X, 칸 (X, Y) 쪽.
	for (int32 X = 0; X <= Size.X; ++X)
	{
		for (int32 Y = 0; Y < Size.Y; ++Y)
		{
			const bool bFront = IsCellSelected(FIntPoint(X, Y));
			if (IsCellSelected(FIntPoint(X - 1, Y)) != bFront)
			{
				AddEdge(X, Y, 1, !bFront);
			}
		}
	}

	return Result;
}

void UBuildingGridWidget::BuildOutlineWalls(UStaticMesh* Mesh)
{
	ABuildingGrid* GridActor = Grid.Get();
	if (!GridActor || !Mesh)
	{
		return;
	}

	// 외곽선이 아닌 변은 넘기지 않으므로 기존 메시가 그대로 남는다. 로컬 호출만.
	TArray<FBuildingGridEdge> OutlineEdges = GetOutlineEdges();
	if (OutlineEdges.IsEmpty())
	{
		return;
	}

	for (FBuildingGridEdge& Edge : OutlineEdges)
	{
		Edge.Mesh = Mesh;
	}
	GridActor->SetEdgeMeshes(OutlineEdges);
}
