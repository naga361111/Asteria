// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Building/Common/BuildingGrid.h"
#include "BuildingGridWidget.generated.h"

class UStaticMesh;

// 레벨 격자의 범위(GridSize)를 선으로 그리는 표시 전용 위젯. WBP_TabMenuWidget 안에 배치한다.
UCLASS()
class ASTERIA_API UBuildingGridWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// SelectedCells를 Grid->ApplyFloorCells()로 반영해 외곽선 벽을 Mesh로 만들고 사라진 외곽선 벽을 지운다. UTabMenuWidget::HandleBuildWallsClicked()가 호출.
	void BuildOutlineWalls(UStaticMesh* Mesh);

protected:
	// 격자 선 색. WBP에서 지정.
	UPROPERTY(EditAnywhere, Category = "Grid")
	FLinearColor LineColor = FLinearColor::White;

	// 확정 영역 색. 미리보기·가리킨 칸은 같은 색에 알파 절반. WBP에서 지정.
	UPROPERTY(EditAnywhere, Category = "Grid")
	FLinearColor SelectionColor = FLinearColor(0.2f, 0.6f, 1.f, 0.5f);

	// 외곽선 색. WBP에서 지정.
	UPROPERTY(EditAnywhere, Category = "Grid")
	FLinearColor OutlineColor = FLinearColor(1.f, 0.8f, 0.2f, 1.f);

	// 제거 미리보기 색. WBP에서 지정.
	UPROPERTY(EditAnywhere, Category = "Grid")
	FLinearColor EraseColor = FLinearColor(0.f, 0.f, 0.f, 0.5f);

	// 열릴 때마다 레벨의 첫 번째 ABuildingGrid를 찾아 Grid에 저장하고, SelectedCells를 Grid의 FloorCells로 다시 읽고, 진행 중인 선택(SelectionAnchor·HoveredCell)을 초기화.
	virtual void NativeConstruct() override;

	// 마우스 아래 칸을 HoveredCell에 저장.
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// HoveredCell을 비운다.
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	// 같은 버튼 클릭 두 번으로 사각형 칸을 왼쪽 = SelectedCells에 추가(겹치면 무시), 오른쪽 = SelectedCells에서 제거. 첫 클릭과 다른 버튼이면 취소.
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// Grid의 현재 GridSize로 선택 칸과 격자 선, SelectedCells의 외곽선을 그린다.
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	// 그릴 격자. 레벨의 첫 번째 ABuildingGrid.
	TWeakObjectPtr<ABuildingGrid> Grid;

	// 첫 클릭으로 정한 모서리 칸. 없으면 선택 중 아님.
	TOptional<FIntPoint> SelectionAnchor;

	// 마우스 아래 칸. 격자 밖이면 없음.
	TOptional<FIntPoint> HoveredCell;

	// SelectionAnchor가 제거용(오른쪽 클릭)이면 true.
	bool bEraseSelection = false;

	// 편집 중인 칸 집합. 열 때 Grid의 FloorCells에서 복사, BuildOutlineWalls()로 반영. 반영하지 않은 편집은 다시 열면 버려짐.
	TSet<FIntPoint> SelectedCells;

	// Rect(Min 포함·Max 미포함) 안 칸 중 SelectedCells에 있는 것이 있으면 true.
	bool OverlapsSelectedCells(const FIntRect& Rect) const;

	// 화면 좌표의 칸. 격자 밖·Grid 무효면 빈 값.
	TOptional<FIntPoint> GetCellAt(const FGeometry& Geometry, const FVector2D& ScreenPos) const;

	// 칸 한 변 길이. Grid 무효면 0.
	float GetCellSize(const FVector2D& LocalSize) const;
};
