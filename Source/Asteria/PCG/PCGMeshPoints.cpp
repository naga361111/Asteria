#include "PCG/PCGMeshPoints.h"

#include "PCGContext.h"
#include "PCGGraphExecutionStateInterface.h"
#include "Data/PCGBasePointData.h"
#include "Helpers/PCGHelpers.h"
#include "Metadata/PCGMetadata.h"

namespace PCGMeshPoints
{
	const FName MeshAttribute(TEXT("Mesh"));

	FTransform GetVolumeOrigin(const FPCGContext* Context, bool bBottomCenter)
	{
		const IPCGGraphExecutionSource* Source = Context->ExecutionSource.Get();
		if (!Source)
		{
			return FTransform::Identity;
		}
		const IPCGGraphExecutionState& State = Source->GetExecutionState();
		const FTransform VolumeTransform = State.GetTransform();
		const FBox Bounds = State.GetLocalSpaceBounds();
		const FVector Local = bBottomCenter ? FVector(Bounds.GetCenter().X, Bounds.GetCenter().Y, Bounds.Min.Z) : Bounds.Min;
		const FVector Origin = VolumeTransform.TransformPosition(Local);
		return FTransform(VolumeTransform.GetRotation(), Origin);
	}

	UPCGBasePointData* MakePointData(FPCGContext* Context, const TArray<FTransform>& Transforms, const TArray<FSoftObjectPath>& Meshes)
	{
		UPCGBasePointData* PointData = FPCGContext::NewPointData_AnyThread(Context);
		PointData->SetNumPoints(Transforms.Num(), /*bInitializeValues=*/false);
		PointData->AllocateProperties(EPCGPointNativeProperties::All);
		FPCGMetadataAttribute<FSoftObjectPath>* MeshAttr =
			PointData->Metadata->FindOrCreateAttribute<FSoftObjectPath>(MeshAttribute, FSoftObjectPath(), false, false);

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
}
