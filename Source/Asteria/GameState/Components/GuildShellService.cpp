// Fill out your copyright notice in the Description page of Project Settings.


#include "GuildShellService.h"

#include "PCG/BuildingLayout.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PCGComponent.h"
#include "PCGGraph.h"

namespace
{
	constexpr int32 MaxShellHalfCells = BuildingLayout::MaxHalfCells;

	// PCG_Building 그래프 파라미터 이름.
	const FName ShellHalfWidthParam(TEXT("HalfWidth"));
	const FName ShellHalfHeightParam(TEXT("HalfHeight"));
}

UGuildShellService::UGuildShellService()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;
}

void UGuildShellService::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGuildShellService, ShellHalfWidth);
	DOREPLIFETIME(UGuildShellService, ShellHalfHeight);
}

void UGuildShellService::BeginPlay()
{
	Super::BeginPlay();

	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	UPCGComponent* PCG = FindShellPCG();
	const UPCGGraphInstance* Graph = PCG ? PCG->GetGraphInstance() : nullptr;
	if (!Graph)
	{
		return;
	}

	// 레벨에 이미 생성돼 있는 크기를 복제 기준으로 삼는다. 여기서 재생성은 하지 않는다.
	const TValueOrError<int32, EPropertyBagResult> HalfWidth = Graph->GetGraphParameter<int32>(ShellHalfWidthParam);
	const TValueOrError<int32, EPropertyBagResult> HalfHeight = Graph->GetGraphParameter<int32>(ShellHalfHeightParam);
	if (HalfWidth.HasValue() && HalfHeight.HasValue())
	{
		ShellHalfWidth = FMath::Clamp(HalfWidth.GetValue(), 0, MaxShellHalfCells);
		ShellHalfHeight = FMath::Clamp(HalfHeight.GetValue(), 0, MaxShellHalfCells);
	}
}

void UGuildShellService::OnRep_ShellSize()
{
	ApplyShellSize();
}

bool UGuildShellService::SetShellSize(int32 HalfWidth, int32 HalfHeight)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return false;
	}
	if (HalfWidth < 0 || HalfWidth > MaxShellHalfCells || HalfHeight < 0 || HalfHeight > MaxShellHalfCells)
	{
		UE_LOG(LogTemp, Warning, TEXT("GuildShellService: half size %dx%d out of range (0~%d)."), HalfWidth, HalfHeight, MaxShellHalfCells);
		return false;
	}

	ShellHalfWidth = HalfWidth;
	ShellHalfHeight = HalfHeight;
	// 서버(호스트)에선 OnRep이 불리지 않으므로 직접 적용한다.
	ApplyShellSize();
	return true;
}

UPCGComponent* UGuildShellService::FindShellPCG() const
{
	TArray<AActor*> VolumeActors;
	UGameplayStatics::GetAllActorsWithTag(this, TEXT("GuildShellVolume"), VolumeActors);
	return VolumeActors.Num() > 0 ? VolumeActors[0]->FindComponentByClass<UPCGComponent>() : nullptr;
}

void UGuildShellService::ApplyShellSize()
{
	UPCGComponent* PCG = FindShellPCG();
	UPCGGraphInstance* Graph = PCG ? PCG->GetGraphInstance() : nullptr;
	if (!Graph)
	{
		UE_LOG(LogTemp, Warning, TEXT("GuildShellService: PCG component on GuildShellVolume not found. Resize skipped."));
		return;
	}

	// 이미 이 크기로 지어졌거나 짓는 중이면 건너뛴다. 클라 접속 시 레벨에 저장된 건물과 같을 때,
	// 그리고 OnRep이 가로·세로 값마다 한 번씩(두 번) 불릴 때 다시 짓지 않게 한다.
	const TValueOrError<int32, EPropertyBagResult> CurrentHalfWidth = Graph->GetGraphParameter<int32>(ShellHalfWidthParam);
	const TValueOrError<int32, EPropertyBagResult> CurrentHalfHeight = Graph->GetGraphParameter<int32>(ShellHalfHeightParam);
	if ((PCG->bGenerated || PCG->IsGenerating())
		&& CurrentHalfWidth.HasValue() && CurrentHalfWidth.GetValue() == ShellHalfWidth
		&& CurrentHalfHeight.HasValue() && CurrentHalfHeight.GetValue() == ShellHalfHeight)
	{
		return;
	}

	if (Graph->SetGraphParameter<int32>(ShellHalfWidthParam, ShellHalfWidth) != EPropertyBagResult::Success
		|| Graph->SetGraphParameter<int32>(ShellHalfHeightParam, ShellHalfHeight) != EPropertyBagResult::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("GuildShellService: graph has no HalfWidth/HalfHeight parameter. Resize skipped."));
		return;
	}

	// 각 머신이 자기 쪽에서 적용하므로 복제되지 않는 로컬 호출을 쓴다.
	PCG->CleanupLocalImmediate(true);
	PCG->GenerateLocal(true);

	UE_LOG(LogTemp, Warning, TEXT("GuildShellService: shell resized to half %dx%d (cells %dx%d)."),
		ShellHalfWidth, ShellHalfHeight, ShellHalfWidth * 2 + 1, ShellHalfHeight * 2 + 1);
}
