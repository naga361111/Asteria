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
	// 현재 외곽선 변 전체에 Mesh를 안쪽 방향으로 넣어 Grid->SetEdgeMeshes()로 보낸다. UTabMenuWidget::HandleBuildWallsClicked()가 호출.
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

	// 열릴 때마다 레벨의 첫 번째 ABuildingGrid를 찾아 Grid에 저장하고 선택 상태를 초기화.
	virtual void NativeConstruct() override;

	// 마우스 아래 칸을 HoveredCell에 저장.
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// HoveredCell을 비운다.
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	// 왼쪽 클릭 두 번으로 SelectedRects에 추가(겹치면 무시), 오른쪽 클릭으로 취소.
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// Grid의 현재 GridSize로 선택 영역과 격자 선, 확정 사각형들의 외곽선을 그린다.
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	// 그릴 격자. 레벨의 첫 번째 ABuildingGrid.
	TWeakObjectPtr<ABuildingGrid> Grid;

	// 첫 클릭으로 정한 모서리 칸. 없으면 선택 중 아님.
	TOptional<FIntPoint> SelectionAnchor;

	// 마우스 아래 칸. 격자 밖이면 없음.
	TOptional<FIntPoint> HoveredCell;

	// 확정된 사각형들(칸 단위, Min 포함·Max 미포함). 서로 겹치지 않음. 확정할 때마다 추가.
	TArray<FIntRect> SelectedRects;

	// Rect가 SelectedRects 중 하나와 칸을 공유하면 true. 변만 맞닿으면 false.
	bool OverlapsSelectedRects(const FIntRect& Rect) const;

	// Cell이 SelectedRects 중 하나라도 포함되면 true.
	bool IsCellSelected(const FIntPoint& Cell) const;

	// 외곽선 변 목록. Vertex·Axis·bFlip(안쪽 향함)만 채우고 Mesh는 null. Grid 무효면 빈 배열. NativePaint()·BuildOutlineWalls()가 호출.
	TArray<FBuildingGridEdge> GetOutlineEdges() const;

	// 화면 좌표의 칸. 격자 밖·Grid 무효면 빈 값.
	TOptional<FIntPoint> GetCellAt(const FGeometry& Geometry, const FVector2D& ScreenPos) const;

	// 칸 한 변 길이. Grid 무효면 0.
	float GetCellSize(const FVector2D& LocalSize) const;
};
