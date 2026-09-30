#pragma once

#include "CoreMinimal.h"
#include "PCGSettings.h"
#include "PCGBuildingSettings.generated.h"

/**
 * PCG_Building의 건물 생성 노드 "Building". 칸 수·문 설정(그래프 파라미터에서 덮어쓰기 핀으로 받음)으로
 * 바닥·외곽 벽·문·모서리 기둥을 배치한다. 배치 계산은 BuildingLayout::FBuilder(BuildingLayout.h).
 * 건물 원점은 실행 소스(볼륨)의 로컬 경계 최소 모서리. 볼륨 위치·회전을 따른다.
 * 출력 핀 Out의 점에는 Mesh 속성(메시 경로)이 달려 Static Mesh Spawner(By Attribute)로 뿌린다.
 */
UCLASS(BlueprintType, ClassGroup = (Procedural))
class ASTERIA_API UPCGBuildingSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("Building")); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCGBuilding", "NodeTitle", "Building"); }
	virtual FText GetNodeTooltipText() const override { return NSLOCTEXT("PCGBuilding", "NodeTooltip", "Places the building floor, outer walls, doors and corner columns for the given cell counts."); }
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Spatial; }
#endif

	// 가로(X)·세로(Y) 가운데 칸을 뺀 한쪽 칸 수(한 칸 300, 칸 수 = 2N+1). 그래프의 HalfWidth·HalfHeight 파라미터를 받는다.
	// 0~BuildingLayout::MaxHalfCells로 자른다. ClampMax는 MaxHalfCells와 같은 값.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable, ClampMin = "0", ClampMax = "15"))
	int32 HalfWidth = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable, ClampMin = "0", ClampMax = "15"))
	int32 HalfHeight = 1;

	// 변마다 문(그래프 파라미터 DoorNegY 등의 bEnabled·Offset). 문 중심 = 벽 중심 + 오프셋×300, 모서리 칸에 닿으면 멈춘다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Doors", meta = (PCG_Overridable))
	bool bDoorNegY = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Doors", meta = (PCG_Overridable))
	int32 DoorNegYOffset = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Doors", meta = (PCG_Overridable))
	bool bDoorPosX = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Doors", meta = (PCG_Overridable))
	int32 DoorPosXOffset = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Doors", meta = (PCG_Overridable))
	bool bDoorPosY = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Doors", meta = (PCG_Overridable))
	int32 DoorPosYOffset = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Doors", meta = (PCG_Overridable))
	bool bDoorNegX = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Doors", meta = (PCG_Overridable))
	int32 DoorNegXOffset = 0;

protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override { return {}; }
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
	virtual FPCGElementPtr CreateElement() const override;
};

class FPCGBuildingElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;

	// 캐시를 끈다. 결과가 볼륨 트랜스폼에 따라 달라지는데 입력 핀이 없어 캐시 키에 안 잡힌다.
	virtual bool IsCacheable(const UPCGSettings* InSettings) const override { return false; }
};
