#include "PCG/PCGCounterSettings.h"

#include "PCG/PCGMeshPoints.h"
#include "PCGContext.h"
#include "PCGPin.h"
#include "Data/PCGBasePointData.h"

TArray<FPCGPinProperties> UPCGCounterSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> Pins;
	Pins.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Point);
	return Pins;
}

FPCGElementPtr UPCGCounterSettings::CreateElement() const
{
	return MakeShared<FPCGCounterElement>();
}

bool FPCGCounterElement::ExecuteInternal(FPCGContext* Context) const
{
	const UPCGCounterSettings* Settings = Context->GetInputSettings<UPCGCounterSettings>();
	check(Settings);

	// 가운데 판 개수는 그래프 파라미터(외부 입력)라 범위를 자른다.
	const int32 MidCount = FMath::Clamp(Settings->MidCount, 0, CounterLayout::MaxMidCount);
	const FTransform Origin = PCGMeshPoints::GetVolumeOrigin(Context, /*bBottomCenter=*/true);
	// 카운터 전체(가로 끝 판 둘 + 가운데 판들, 세로 EndDepth)의 가운데가 볼륨 바닥면 가운데에 오게 민다.
	const FVector Center(CounterLayout::EndWidth + MidCount * CounterLayout::Mid * 0.5, CounterLayout::EndDepth * 0.5, 0.0);

	// 모두 yaw 0. 왼쪽 끝 판 피벗(오른쪽 끝)에서 가운데 판을 이어 붙이고 마지막 끝에 오른쪽 끝 판.
	TArray<FTransform> Transforms;
	TArray<FSoftObjectPath> Meshes;
	const auto Add = [&](double X, const FSoftObjectPath& Mesh)
	{
		Transforms.Add(FTransform(FVector(X, CounterLayout::EndDepth, 0.0) - Center) * Origin);
		Meshes.Add(Mesh);
	};
	Add(CounterLayout::EndWidth, CounterLayout::Mesh::Left);
	for (int32 Index = 0; Index < MidCount; ++Index)
	{
		Add(CounterLayout::EndWidth + Index * CounterLayout::Mid, CounterLayout::Mesh::Mid);
	}
	Add(CounterLayout::EndWidth + MidCount * CounterLayout::Mid, CounterLayout::Mesh::Right);

	FPCGTaggedData& Out = Context->OutputData.TaggedData.Emplace_GetRef();
	Out.Data = PCGMeshPoints::MakePointData(Context, Transforms, Meshes);
	Out.Pin = PCGPinConstants::DefaultOutputLabel;
	return true;
}
