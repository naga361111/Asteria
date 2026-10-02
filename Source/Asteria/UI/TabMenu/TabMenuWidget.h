// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TabMenuWidget.generated.h"

class UBuildingEdgeMeshData;
class UComboBoxString;
class UButton;
class UBuildingGridWidget;

/**
 * 탭 키로 여닫는 화면 UI의 C++ 베이스. 내용은 WBP에서 구성한다.
 */
UCLASS()
class ASTERIA_API UTabMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// 콤보박스 두 개에 채울 메시 목록 에셋. WBP_TabMenuWidget에서 지정.
	UPROPERTY(EditAnywhere, Category = "Placement")
	TObjectPtr<UBuildingEdgeMeshData> EdgeMeshData;

	// 외곽선 메시 선택. 옵션 인덱스 = EdgeMeshData->OutlineMeshes 인덱스.
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UComboBoxString> OutlineMeshComboBox;

	// 내부 메시 선택. 옵션 인덱스 = EdgeMeshData->InteriorMeshes 인덱스.
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UComboBoxString> InteriorMeshComboBox;

	// 벽 생성 버튼.
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> BuildWallsButton;

	// 벽 변을 그리는 격자 위젯. WBP_TabMenuWidget에 배치된 WBP_BuildingGrid 인스턴스.
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBuildingGridWidget> BuildingGridWidget;

	// 콤보박스를 채우고 버튼을 바인딩. 엔진이 1회 호출.
	virtual void NativeOnInitialized() override;

	// 두 콤보 인덱스를 BuildingGridWidget->SetMeshChoices()로 넘김. OnSelectionChanged(FOnSelectionChangedEvent) 바인딩 대상이라 UFUNCTION 필수.
	UFUNCTION()
	void HandleMeshSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	// BuildingGridWidget->BuildWalls() 호출. 동적 델리게이트(OnClicked) 대상이라 UFUNCTION 필수.
	UFUNCTION()
	void HandleBuildWallsClicked();
};
