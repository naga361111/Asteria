#include "PCG/PCGBuildingSettings.h"

#include "PCG/BuildingLayout.h"
#include "PCG/PCGMeshPoints.h"
#include "PCGContext.h"
#include "PCGPin.h"
#include "Data/PCGBasePointData.h"

namespace
{
	FBuildingDoor MakeBuildingDoor(bool bEnabled, int32 Offset)
	{
		FBuildingDoor Door;
		Door.bEnabled = bEnabled;
		Door.Offset = Offset;
		return Door;
	}
}

TArray<FPCGPinProperties> UPCGBuildingSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> Pins;
	Pins.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Point);
	return Pins;
}

FPCGElementPtr UPCGBuildingSettings::CreateElement() const
{
	return MakeShared<FPCGBuildingElement>();
}

bool FPCGBuildingElement::ExecuteInternal(FPCGContext* Context) const
{
	const UPCGBuildingSettings* Settings = Context->GetInputSettings<UPCGBuildingSettings>();
	check(Settings);

	const FBuildingDoor Doors[4] = {
		MakeBuildingDoor(Settings->bDoorNegY, Settings->DoorNegYOffset),
		MakeBuildingDoor(Settings->bDoorPosX, Settings->DoorPosXOffset),
		MakeBuildingDoor(Settings->bDoorPosY, Settings->DoorPosYOffset),
		MakeBuildingDoor(Settings->bDoorNegX, Settings->DoorNegXOffset),
	};
	// 한쪽 칸 수는 그래프 파라미터(외부 입력)라 범위를 자른다.
	const BuildingLayout::FBuilder Building(
		FMath::Clamp(Settings->HalfWidth, 0, BuildingLayout::MaxHalfCells),
		FMath::Clamp(Settings->HalfHeight, 0, BuildingLayout::MaxHalfCells),
		Doors);

	const FTransform Origin = PCGMeshPoints::GetVolumeOrigin(Context);
	TArray<FTransform> Transforms;
	Transforms.Reserve(Building.Transforms.Num());
	for (const FTransform& Local : Building.Transforms)
	{
		Transforms.Add(Local * Origin);
	}

	FPCGTaggedData& Out = Context->OutputData.TaggedData.Emplace_GetRef();
	Out.Data = PCGMeshPoints::MakePointData(Context, Transforms, Building.Meshes);
	Out.Pin = PCGPinConstants::DefaultOutputLabel;
	return true;
}
