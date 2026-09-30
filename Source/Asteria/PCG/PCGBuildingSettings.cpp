#include "PCG/PCGBuildingSettings.h"

#include "PCG/BuildingLayout.h"
#include "PCGContext.h"
#include "PCGGraphExecutionStateInterface.h"
#include "PCGPin.h"
#include "Data/PCGBasePointData.h"
#include "Helpers/PCGHelpers.h"
#include "Metadata/PCGMetadata.h"

namespace
{
	const FName BuildingMeshAttribute(TEXT("Mesh"));

	// 점마다 Mesh 속성을 단 점 데이터.
	UPCGBasePointData* MakeBuildingPointData(FPCGContext* Context, const TArray<FTransform>& Transforms, const TArray<FSoftObjectPath>& Meshes)
	{
		UPCGBasePointData* PointData = FPCGContext::NewPointData_AnyThread(Context);
		PointData->SetNumPoints(Transforms.Num(), /*bInitializeValues=*/false);
		PointData->AllocateProperties(EPCGPointNativeProperties::All);
		FPCGMetadataAttribute<FSoftObjectPath>* MeshAttr =
			PointData->Metadata->FindOrCreateAttribute<FSoftObjectPath>(BuildingMeshAttribute, FSoftObjectPath(), false, false);

		FPCGPointValueRanges Ranges(PointData, /*bAllocate=*/false);
		for (int32 Index = 0; Index < Transforms.Num(); ++Index)
		{
			Ranges.TransformRange[Index] = Transforms[Index];
			Ranges.DensityRange[Index] = 1.0f;
			Ranges.BoundsMinRange[Index] = FVector(-1.0);
			Ranges.BoundsMaxRange[Index] = FVector(1.0);
			Ranges.ColorRange[Index] = FVector4(1.0);
			Ranges.SteepnessRange[Index] = 1.0f;
			Ranges.SeedRange[Index] = PCGHelpers::ComputeSeedFromPosition(Transforms[Index].GetLocation());
			Ranges.MetadataEntryRange[Index] = PCGInvalidEntryKey;
			PointData->Metadata->InitializeOnSet(Ranges.MetadataEntryRange[Index]);
			MeshAttr->SetValue(Ranges.MetadataEntryRange[Index], Meshes[Index]);
		}
		return PointData;
	}

	// 건물 (0,0,0) = 볼륨 로컬 경계 최소 모서리. 볼륨 위치·회전은 따르고 스케일은 빼 메시 크기를 유지한다.
	// 실행 소스가 없으면 항등.
	FTransform GetBuildingOrigin(const FPCGContext* Context)
	{
		const IPCGGraphExecutionSource* Source = Context->ExecutionSource.Get();
		if (!Source)
		{
			return FTransform::Identity;
		}
		const IPCGGraphExecutionState& State = Source->GetExecutionState();
		const FTransform VolumeTransform = State.GetTransform();
		const FVector Origin = VolumeTransform.TransformPosition(State.GetLocalSpaceBounds().Min);
		return FTransform(VolumeTransform.GetRotation(), Origin);
	}

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
	// 칸 수는 그래프 파라미터(외부 입력)라 범위를 자른다.
	const BuildingLayout::FBuilder Building(
		FMath::Clamp(Settings->Width, 1, BuildingLayout::MaxCells),
		FMath::Clamp(Settings->Height, 1, BuildingLayout::MaxCells),
		Doors);

	const FTransform Origin = GetBuildingOrigin(Context);
	TArray<FTransform> Transforms;
	Transforms.Reserve(Building.Transforms.Num());
	for (const FTransform& Local : Building.Transforms)
	{
		Transforms.Add(Local * Origin);
	}

	FPCGTaggedData& Out = Context->OutputData.TaggedData.Emplace_GetRef();
	Out.Data = MakeBuildingPointData(Context, Transforms, Building.Meshes);
	Out.Pin = PCGPinConstants::DefaultOutputLabel;
	return true;
}
