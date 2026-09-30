#pragma once

#include "CoreMinimal.h"
#include "PCGSettings.h"
#include "PCGCounterSettings.generated.h"

// 카운터(Hearthvale Tabletop 세트) 배치. 끝 판 Lt_150 + 가운데 판 Mid_300 × N + 끝 판 Rt_150.
namespace CounterLayout
{
	// 가운데 판 개수 상한.
	constexpr int32 MaxMidCount = 10;
	// 가운데 판 한 개 길이.
	constexpr double Mid = 300.0;
	// 끝 판(Lt_150/Rt_150)의 공칭 폭. 카운터 몸통 시작 X.
	constexpr double EndWidth = 150.0;
	// 끝 판이 -Y로 꺾여 내려가는 깊이. 카운터 몸통 Y.
	constexpr double EndDepth = 170.0;

	// 메시. 로컬 +Y 면이 손님 쪽, 높이 132. 배치 코드가 각 메시의 피벗·치수에 맞춰져 있어 메시를 바꾸면 배치 코드도 봐야 한다.
	namespace Mesh
	{
		// 왼쪽 끝 판(피벗 오른쪽 끝, -X로 뻗고 -Y로 꺾임)
		inline const FSoftObjectPath Left(TEXT("/Game/Hearthvale/Meshes/Furniture/SM_Tabletop_Lt_150.SM_Tabletop_Lt_150"));
		// 가운데 판(피벗 왼쪽 끝, +X로 300, 깊이 -Y 107)
		inline const FSoftObjectPath Mid(TEXT("/Game/Hearthvale/Meshes/Furniture/SM_Tabletop_Mid_300.SM_Tabletop_Mid_300"));
		// 오른쪽 끝 판(피벗 왼쪽 끝, +X로 뻗고 -Y로 꺾임)
		inline const FSoftObjectPath Right(TEXT("/Game/Hearthvale/Meshes/Furniture/SM_Tabletop_Rt_150.SM_Tabletop_Rt_150"));
	}
}

/**
 * PCG_Counter의 카운터 생성 노드 "Counter". 가운데 판 개수(그래프 파라미터에서 덮어쓰기 핀으로 받음)로
 * 같은 패턴을 유지한 채 카운터 길이를 바꾼다.
 * 카운터는 실행 소스(볼륨) 바닥면 가운데에 가운데를 맞춰 놓는다. 볼륨 위치·회전을 따르고 크기와는 무관하다.
 * 출력 핀 Out의 점에는 Mesh 속성(메시 경로)이 달려 Static Mesh Spawner(By Attribute)로 뿌린다.
 */
UCLASS(BlueprintType, ClassGroup = (Procedural))
class ASTERIA_API UPCGCounterSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("Counter")); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCGCounter", "NodeTitle", "Counter"); }
	virtual FText GetNodeTooltipText() const override { return NSLOCTEXT("PCGCounter", "NodeTooltip", "Places a counter from the Tabletop set: left end, the given number of middle pieces, right end."); }
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Spatial; }
#endif

	// 가운데 판 개수. 그래프의 MidCount 파라미터를 받는다. 0이면 끝 판 둘만 붙은 가장 작은 카운터.
	// 0~CounterLayout::MaxMidCount로 자른다. ClampMax는 MaxMidCount와 같은 값.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable, ClampMin = "0", ClampMax = "10"))
	int32 MidCount = 2;

protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override { return {}; }
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
	virtual FPCGElementPtr CreateElement() const override;
};

class FPCGCounterElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;

	// 캐시를 끈다. 결과가 볼륨 트랜스폼에 따라 달라지는데 입력 핀이 없어 캐시 키에 안 잡힌다.
	virtual bool IsCacheable(const UPCGSettings* InSettings) const override { return false; }
};
