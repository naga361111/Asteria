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

	// 탭 메뉴를 열 때마다 진행 중인 선택을 초기화. 칸은 격자(FloorCells)에서 읽음.
	if (const ABuildingGrid* GridActor = Grid.Get())
	{
		SelectedCells = GridActor->FloorCells;
	}
	else
	{
		SelectedCells.Reset();
	}
	SelectionAnchor.Reset();
	HoveredCell.Reset();

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

bool UBuildingGridWidget::OverlapsSelectedCells(const FIntRect& Rect) const
{
	for (int32 Y = Rect.Min.Y; Y < Rect.Max.Y; ++Y)
	{
		for (int32 X = Rect.Min.X; X < Rect.Max.X; ++X)
		{
			if (SelectedCells.Contains(FIntPoint(X, Y)))
			{
				return true;
			}
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
	const FKey Button = InMouseEvent.GetEffectingButton();
	if (Button != EKeys::LeftMouseButton && Button != EKeys::RightMouseButton)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	const TOptional<FIntPoint> CellPos = GetCellAt(InGeometry, InMouseEvent.GetScreenSpacePosition());
	if (!CellPos.IsSet())
	{
		return FReply::Unhandled();
	}

	const bool bErase = Button == EKeys::RightMouseButton;
	if (!SelectionAnchor.IsSet())
	{
		// 첫 모서리. 왼쪽 = 추가, 오른쪽 = 제거.
		SelectionAnchor = CellPos;
		bEraseSelection = bErase;
		return FReply::Handled();
	}

	if (bErase != bEraseSelection)
	{
		// 첫 클릭과 다른 버튼이면 취소.
		SelectionAnchor.Reset();
		return FReply::Handled();
	}

	// 반대 모서리로 확정.
	const FIntPoint A = SelectionAnchor.GetValue();
	const FIntPoint B = CellPos.GetValue();
	const FIntRect Rect(A.ComponentMin(B), A.ComponentMax(B) + FIntPoint(1, 1));
	if (!bEraseSelection && OverlapsSelectedCells(Rect))
	{
		// 추가가 기존 칸과 겹치면 무시하고 첫 모서리 유지.
		return FReply::Handled();
	}

	for (int32 Y = Rect.Min.Y; Y < Rect.Max.Y; ++Y)
	{
		for (int32 X = Rect.Min.X; X < Rect.Max.X; ++X)
		{
			if (bEraseSelection)
			{
				SelectedCells.Remove(FIntPoint(X, Y));
			}
			else
			{
				SelectedCells.Add(FIntPoint(X, Y));
			}
		}
	}
	SelectionAnchor.Reset();
	return FReply::Handled();
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

	// 선택 칸들.
	for (const FIntPoint& Selected : SelectedCells)
	{
		DrawRect(FIntRect(Selected, Selected + FIntPoint(1, 1)), SelectionColor);
	}

	if (HoveredCell.IsSet())
	{
		const FIntPoint H = HoveredCell.GetValue();
		if (SelectionAnchor.IsSet())
		{
			// 미리보기 사각형. 제거면 EraseColor, 추가가 기존 칸과 겹치면 빨강.
			const FIntPoint A = SelectionAnchor.GetValue();
			const FIntRect Preview(A.ComponentMin(H), A.ComponentMax(H) + FIntPoint(1, 1));
			if (bEraseSelection)
			{
				DrawRect(Preview, EraseColor);
			}
			else
			{
				DrawRect(Preview, OverlapsSelectedCells(Preview) ? FLinearColor(1.f, 0.f, 0.f, PreviewColor.A) : PreviewColor);
			}
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
	for (const FBuildingGridEdge& Edge : ABuildingGrid::GetOutlineEdges(SelectedCells, Size))
	{
		const FIntPoint End = Edge.Vertex + (Edge.Axis == 0 ? FIntPoint(1, 0) : FIntPoint(0, 1));
		Points[0] = FVector2D(Cell * Edge.Vertex.X, Cell * Edge.Vertex.Y);
		Points[1] = FVector2D(Cell * End.X, Cell * End.Y);
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 3, AllottedGeometry.ToPaintGeometry(), Points, ESlateDrawEffect::None, OutlineColor, true, 3.f);
	}

	return LayerId + 3;
}

void UBuildingGridWidget::BuildOutlineWalls(UStaticMesh* Mesh)
{
	ABuildingGrid* GridActor = Grid.Get();
	if (!GridActor || !Mesh)
	{
		return;
	}

	GridActor->ApplyFloorCells(SelectedCells, Mesh);
}
