// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Building/Common/BuildingGrid.h"
#include "BuildingGridWidget.generated.h"

class UBuildingEdgeMeshData;
class UStaticMesh;
struct FBuildingEdgeMeshEntry;

// 레벨 격자(GridSize)에 벽 변을 그리고 지우는 편집 위젯. WBP_TabMenuWidget 안에 배치한다.
UCLASS()
class ASTERIA_API UBuildingGridWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// MeshData·OutlineMeshIndex·InteriorMeshIndex를 설정. UTabMenuWidget::HandleMeshSelectionChanged()가 호출.
	void SetMeshChoices(UBuildingEdgeMeshData* InMeshData, int32 InOutlineMeshIndex, int32 InInteriorMeshIndex);

	// WallEdges를 Grid->ApplyWallEdges()로 반영. UTabMenuWidget::HandleBuildWallsClicked()가 호출.
	void BuildWalls();

protected:
	// 격자 선 색. WBP에서 지정.
	UPROPERTY(EditAnywhere, Category = "Grid")
	FLinearColor LineColor = FLinearColor::White;

	// 벽으로 둘러싸인 바닥 칸 색. WBP에서 지정.
	UPROPERTY(EditAnywhere, Category = "Grid")
	FLinearColor FloorColor = FLinearColor(0.2f, 0.6f, 1.f, 0.5f);

	// 그리기 획·가리킨 변 미리보기 색. WBP에서 지정.
	UPROPERTY(EditAnywhere, Category = "Grid")
	FLinearColor PreviewColor = FLinearColor(1.f, 1.f, 1.f, 0.5f);

	// 지우기 획 미리보기 색. WBP에서 지정.
	UPROPERTY(EditAnywhere, Category = "Grid")
	FLinearColor EraseColor = FLinearColor(0.f, 0.f, 0.f, 0.5f);

	// MeshData 목록에 없는 메시를 가진 변 색. 닫히지 않은 변은 메시 색을 흐리게. WBP에서 지정.
	UPROPERTY(EditAnywhere, Category = "Grid")
	FLinearColor OpenColor = FLinearColor::Red;

	// 열릴 때마다 레벨의 첫 번째 ABuildingGrid를 찾아 Grid에 저장하고, WallEdges를 Grid->Edges에서 다시 읽고, DragStart·HoveredPos를 비운다.
	virtual void NativeConstruct() override;

	// 마우스 아래 격자 칸 단위 좌표를 HoveredPos에 저장.
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// HoveredPos를 비운다.
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	// 왼쪽 = 그리기, 오른쪽 = 지우기 획 시작. 획 중 다른 버튼이면 취소.
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// 획 확정.
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// 바닥 칸, 격자 선, 벽 변(메시 색), 획 미리보기를 그린다.
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	// 그릴 격자. 레벨의 첫 번째 ABuildingGrid.
	TWeakObjectPtr<ABuildingGrid> Grid;

	// 메시 목록. SetMeshChoices()가 설정.
	UPROPERTY(Transient)
	TObjectPtr<UBuildingEdgeMeshData> MeshData;

	// 외곽선 콤보에서 고른 MeshData->OutlineMeshes 인덱스.
	int32 OutlineMeshIndex = INDEX_NONE;

	// 내부 콤보에서 고른 MeshData->InteriorMeshes 인덱스.
	int32 InteriorMeshIndex = INDEX_NONE;

	// 편집 중인 벽 변(Vertex·Axis·Mesh·bFlip). 열 때 Grid->Edges 중 Mesh 있는 변에서 복사, BuildWalls()로 반영. 반영하지 않은 편집은 다시 열면 버려짐.
	// Mesh는 그린 시점의 콤보 메시로 확정. 콤보를 바꿔도 바뀌지 않음.
	UPROPERTY(Transient)
	TArray<FBuildingGridEdge> WallEdges;

	// 버튼을 누른 격자 칸 단위 좌표. 없으면 획 진행 중 아님.
	TOptional<FVector2D> DragStart;

	// DragStart가 오른쪽 버튼(지우기)이면 true.
	bool bErasing = false;

	// 마우스 아래 격자 칸 단위 좌표. 위젯 밖이면 없음.
	TOptional<FVector2D> HoveredPos;

	// 화면 좌표의 격자 칸 단위 좌표(위젯 로컬 / GetCellSize). Grid 무효거나 칸 크기 0이면 빈 값. 격자 밖도 값 반환.
	TOptional<FVector2D> GetGridPos(const FGeometry& Geometry, const FVector2D& ScreenPos) const;

	// 획의 변 목록(Vertex·Axis만 채움, Mesh null). 시작·끝 꼭짓점이 같으면 End에서 가장 가까운 변 하나, 아니면 우세 축 직선.
	TArray<FBuildingGridEdge> GetStrokeEdges(const FVector2D& Start, const FVector2D& End) const;

	// 새로 그리는 변에 기록할 메시. Interior면 내부 콤보, 그 외(Outline·Open)는 외곽선 콤보. MeshData 없음·인덱스 무효면 nullptr.
	UStaticMesh* GetComboMesh(EBuildingEdgeKind Kind) const;

	// Mesh의 엔트리(외곽선 목록 먼저, 다음 내부 목록). 없으면 nullptr. NativePaint()가 색을 고를 때 호출.
	const FBuildingEdgeMeshEntry* FindMeshEntry(const UStaticMesh* Mesh) const;

	// 칸 한 변 길이. Grid 무효면 0.
	float GetCellSize(const FVector2D& LocalSize) const;
};
