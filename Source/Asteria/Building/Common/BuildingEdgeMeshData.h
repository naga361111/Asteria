// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BuildingEdgeMeshData.generated.h"

class UStaticMesh;

// 격자 변에 놓을 수 있는 메시 목록. 1인칭 배치와 탭 메뉴가 같은 에셋을 쓴다.
UCLASS()
class ASTERIA_API UBuildingEdgeMeshData : public UDataAsset
{
	GENERATED_BODY()

public:
	// 변에 놓을 메시들. AAsteriaPlayer·UTabMenuWidget이 읽는다.
	UPROPERTY(EditAnywhere, Category = "Placement")
	TArray<TObjectPtr<UStaticMesh>> EdgeMeshes;
};
