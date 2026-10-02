// Fill out your copyright notice in the Description page of Project Settings.


#include "TabMenuWidget.h"
#include "Components/ComboBoxString.h"
#include "Components/Button.h"
#include "Building/Common/BuildingEdgeMeshData.h"
#include "UI/TabMenu/BuildingGridWidget.h"

void UTabMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 옵션은 목록과 같은 순서로 채워 콤보박스 인덱스를 그대로 메시 인덱스로 쓴다.
	auto FillComboBox = [](UComboBoxString* ComboBox, const TArray<FBuildingEdgeMeshEntry>& Entries)
	{
		for (const FBuildingEdgeMeshEntry& Entry : Entries)
		{
			ComboBox->AddOption(GetNameSafe(Entry.Mesh));
		}
		if (Entries.Num() > 0)
		{
			ComboBox->SetSelectedIndex(0);
		}
	};

	OutlineMeshComboBox->ClearOptions();
	InteriorMeshComboBox->ClearOptions();
	if (EdgeMeshData == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("TabMenu: EdgeMeshData not set. Mesh combo box stays empty."));
	}
	else
	{
		FillComboBox(OutlineMeshComboBox, EdgeMeshData->OutlineMeshes);
		FillComboBox(InteriorMeshComboBox, EdgeMeshData->InteriorMeshes);
	}

	// 초기 선택을 넘긴 뒤 바인딩해 채우는 중 핸들러가 중복 호출되지 않게 한다.
	HandleMeshSelectionChanged(FString(), ESelectInfo::Direct);
	OutlineMeshComboBox->OnSelectionChanged.AddDynamic(this, &UTabMenuWidget::HandleMeshSelectionChanged);
	InteriorMeshComboBox->OnSelectionChanged.AddDynamic(this, &UTabMenuWidget::HandleMeshSelectionChanged);

	BuildWallsButton->OnClicked.AddDynamic(this, &UTabMenuWidget::HandleBuildWallsClicked);
}

void UTabMenuWidget::HandleMeshSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	BuildingGridWidget->SetMeshChoices(EdgeMeshData, OutlineMeshComboBox->GetSelectedIndex(), InteriorMeshComboBox->GetSelectedIndex());
}

void UTabMenuWidget::HandleBuildWallsClicked()
{
	// 로컬 호출만. 서버 권위는 추후 일괄 적용.
	BuildingGridWidget->BuildWalls();
}
