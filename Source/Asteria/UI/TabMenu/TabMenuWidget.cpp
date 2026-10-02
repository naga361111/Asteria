// Fill out your copyright notice in the Description page of Project Settings.


#include "TabMenuWidget.h"
#include "Components/ComboBoxString.h"
#include "Components/Button.h"
#include "Building/Common/BuildingEdgeMeshData.h"
#include "UI/TabMenu/BuildingGridWidget.h"

void UTabMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 옵션은 EdgeMeshes와 같은 순서로 채워 콤보박스 인덱스를 그대로 메시 인덱스로 쓴다.
	EdgeMeshComboBox->ClearOptions();
	if (EdgeMeshData == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("TabMenu: EdgeMeshData not set. Mesh combo box stays empty."));
	}
	else
	{
		for (const TObjectPtr<UStaticMesh>& EdgeMesh : EdgeMeshData->EdgeMeshes)
		{
			EdgeMeshComboBox->AddOption(GetNameSafe(EdgeMesh));
		}
		if (EdgeMeshData->EdgeMeshes.Num() > 0)
		{
			EdgeMeshComboBox->SetSelectedIndex(0);
		}
	}

	BuildWallsButton->OnClicked.AddDynamic(this, &UTabMenuWidget::HandleBuildWallsClicked);
}

void UTabMenuWidget::HandleBuildWallsClicked()
{
	if (EdgeMeshData == nullptr)
	{
		return;
	}
	const int32 SelectedIndex = EdgeMeshComboBox->GetSelectedIndex();
	if (!EdgeMeshData->EdgeMeshes.IsValidIndex(SelectedIndex))
	{
		return;
	}
	UStaticMesh* SelectedMesh = EdgeMeshData->EdgeMeshes[SelectedIndex];
	if (SelectedMesh == nullptr)
	{
		return;
	}

	// 로컬 호출만. 서버 권위는 추후 일괄 적용.
	BuildingGridWidget->BuildOutlineWalls(SelectedMesh);
}
