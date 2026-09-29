#include "Building/PCGGuildShellSettings.h"

#include "Building/GuildShellBuilder.h"
#include "PCGContext.h"
#include "PCGPin.h"
#include "Data/PCGBasePointData.h"
#include "Helpers/PCGHelpers.h"
#include "Metadata/PCGMetadata.h"

namespace
{
	const FName MeshAttribute(TEXT("Mesh"));
	const FName NoCollisionPin(TEXT("NoCollision"));
	const FName LightsPin(TEXT("Lights"));
	const FName SconceLightsPin(TEXT("SconceLights"));

	// Meshes가 있으면 점마다 Mesh 속성을 단다.
	UPCGBasePointData* MakePointData(FPCGContext* Context, const TArray<FTransform>& Transforms, const TArray<FSoftObjectPath>* Meshes)
	{
		UPCGBasePointData* PointData = FPCGContext::NewPointData_AnyThread(Context);
		PointData->SetNumPoints(Transforms.Num(), /*bInitializeValues=*/false);
		PointData->AllocateProperties(EPCGPointNativeProperties::All);
		FPCGMetadataAttribute<FSoftObjectPath>* MeshAttr = Meshes
			? PointData->Metadata->FindOrCreateAttribute<FSoftObjectPath>(MeshAttribute, FSoftObjectPath(), false, false)
			: nullptr;

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
			if (MeshAttr)
			{
				PointData->Metadata->InitializeOnSet(Ranges.MetadataEntryRange[Index]);
				MeshAttr->SetValue(Ranges.MetadataEntryRange[Index], (*Meshes)[Index]);
			}
		}
		return PointData;
	}

	FGuildShellDoor MakeDoor(bool bEnabled, int32 Offset)
	{
		FGuildShellDoor Door;
		Door.bEnabled = bEnabled;
		Door.Offset = Offset;
		return Door;
	}
}

TArray<FPCGPinProperties> UPCGGuildShellSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> Pins;
	Pins.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Point);
	Pins.Emplace(NoCollisionPin, EPCGDataType::Point);
	Pins.Emplace(LightsPin, EPCGDataType::Point);
	Pins.Emplace(SconceLightsPin, EPCGDataType::Point);
	return Pins;
}

FPCGElementPtr UPCGGuildShellSettings::CreateElement() const
{
	return MakeShared<FPCGGuildShellElement>();
}

bool FPCGGuildShellElement::ExecuteInternal(FPCGContext* Context) const
{
	const UPCGGuildShellSettings* Settings = Context->GetInputSettings<UPCGGuildShellSettings>();
	check(Settings);

	const FGuildShellDoor Doors[4] = {
		MakeDoor(Settings->bDoorNegY, Settings->DoorNegYOffset),
		MakeDoor(Settings->bDoorPosX, Settings->DoorPosXOffset),
		MakeDoor(Settings->bDoorPosY, Settings->DoorPosYOffset),
		MakeDoor(Settings->bDoorNegX, Settings->DoorNegXOffset),
	};
	const FGuildShellBuilder Shell(
		FMath::Clamp(Settings->ShellWidth, 1, UPCGGuildShellSettings::MaxCells),
		FMath::Clamp(Settings->ShellHeight, 1, UPCGGuildShellSettings::MaxCells),
		Doors);

	auto Emit = [Context](FName Pin, const TArray<FTransform>& Transforms, const TArray<FSoftObjectPath>* Meshes)
	{
		FPCGTaggedData& Out = Context->OutputData.TaggedData.Emplace_GetRef();
		Out.Data = MakePointData(Context, Transforms, Meshes);
		Out.Pin = Pin;
	};
	Emit(PCGPinConstants::DefaultOutputLabel, Shell.Blocking.Transforms, &Shell.Blocking.Meshes);
	Emit(NoCollisionPin, Shell.NoCollision.Transforms, &Shell.NoCollision.Meshes);
	Emit(LightsPin, Shell.Lights, nullptr);
	Emit(SconceLightsPin, Shell.SconceLights, nullptr);
	return true;
}
