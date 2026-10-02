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
	// 콤보박스에 채울 메시 목록 에셋. WBP_TabMenuWidget에서 지정.
	UPROPERTY(EditAnywhere, Category = "Placement")
	TObjectPtr<UBuildingEdgeMeshData> EdgeMeshData;

	// 외곽선에 넣을 메시 선택. 옵션 인덱스 = EdgeMeshData->EdgeMeshes 인덱스.
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UComboBoxString> EdgeMeshComboBox;

	// 벽 생성 버튼.
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> BuildWallsButton;

	// 외곽선을 가진 격자 위젯. WBP_TabMenuWidget에 배치된 WBP_BuildingGrid 인스턴스.
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBuildingGridWidget> BuildingGridWidget;

	// 콤보박스를 채우고 버튼을 바인딩. 엔진이 1회 호출.
	virtual void NativeOnInitialized() override;

	// 선택 메시로 BuildingGridWidget->BuildOutlineWalls() 호출. 동적 델리게이트(OnClicked) 대상이라 UFUNCTION 필수.
	UFUNCTION()
	void HandleBuildWallsClicked();
};
