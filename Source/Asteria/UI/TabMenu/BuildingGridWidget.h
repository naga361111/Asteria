// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BuildingGridWidget.generated.h"

class ABuildingGrid;

// 레벨 격자의 범위(GridSize)를 선으로 그리는 표시 전용 위젯. WBP_TabMenuWidget 안에 배치한다.
UCLASS()
class ASTERIA_API UBuildingGridWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// 격자 선 색. WBP에서 지정.
	UPROPERTY(EditAnywhere, Category = "Grid")
	FLinearColor LineColor = FLinearColor::White;

	// 열릴 때마다 레벨의 첫 번째 ABuildingGrid를 찾아 Grid에 저장.
	virtual void NativeConstruct() override;

	// Grid의 현재 GridSize로 격자 선을 그린다.
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	// 그릴 격자. 레벨의 첫 번째 ABuildingGrid.
	TWeakObjectPtr<ABuildingGrid> Grid;
};
