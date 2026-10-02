// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildingGridWidget.h"
#include "EngineUtils.h"
#include "Rendering/DrawElements.h"
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
	const FVector2D LocalSize = AllottedGeometry.GetLocalSize();
	const float Cell = FMath::Min(LocalSize.X / Size.X, LocalSize.Y / Size.Y);
	const float Width = Cell * Size.X;
	const float Height = Cell * Size.Y;

	TArray<FVector2D> Points;
	Points.SetNum(2);

	// 가로선 GridSize.Y+1개.
	for (int32 Y = 0; Y <= Size.Y; ++Y)
	{
		Points[0] = FVector2D(0.f, Cell * Y);
		Points[1] = FVector2D(Width, Cell * Y);
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(), Points, ESlateDrawEffect::None, LineColor);
	}

	// 세로선 GridSize.X+1개.
	for (int32 X = 0; X <= Size.X; ++X)
	{
		Points[0] = FVector2D(Cell * X, 0.f);
		Points[1] = FVector2D(Cell * X, Height);
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(), Points, ESlateDrawEffect::None, LineColor);
	}

	return LayerId + 1;
}
