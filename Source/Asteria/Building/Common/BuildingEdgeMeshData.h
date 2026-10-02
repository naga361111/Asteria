// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BuildingEdgeMeshData.generated.h"

class UStaticMesh;

// 메시 하나와 위젯 표시 색.
USTRUCT()
struct FBuildingEdgeMeshEntry
{
	GENERATED_BODY()

	// 변에 놓을 메시.
	UPROPERTY(EditAnywhere, Category = "Placement")
	TObjectPtr<UStaticMesh> Mesh;

	// UBuildingGridWidget이 이 메시의 변을 칠할 색.
	UPROPERTY(EditAnywhere, Category = "Placement")
	FLinearColor Color = FLinearColor::White;
};

// 격자 변에 놓을 수 있는 메시 목록. 1인칭 배치와 탭 메뉴가 같은 에셋을 쓴다.
UCLASS()
class ASTERIA_API UBuildingEdgeMeshData : public UDataAsset
{
	GENERATED_BODY()

public:
	// 외곽선 변에 놓을 수 있는 메시. AAsteriaPlayer·UTabMenuWidget·UBuildingGridWidget이 읽는다.
	UPROPERTY(EditAnywhere, Category = "Placement")
	TArray<FBuildingEdgeMeshEntry> OutlineMeshes;

	// 내부 변에 놓을 수 있는 메시. AAsteriaPlayer·UTabMenuWidget·UBuildingGridWidget이 읽는다.
	UPROPERTY(EditAnywhere, Category = "Placement")
	TArray<FBuildingEdgeMeshEntry> InteriorMeshes;
};
