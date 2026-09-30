// Fill out your copyright notice in the Description page of Project Settings.


#include "GuildShellService.h"

#include "PCG/BuildingLayout.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PCGComponent.h"
#include "PCGGraph.h"

namespace
{
	constexpr int32 MaxShellCells = BuildingLayout::MaxCells;

	// PCG_Building 그래프 파라미터 이름.
	const FName ShellWidthParam(TEXT("Width"));
	const FName ShellHeightParam(TEXT("Height"));
}

UGuildShellService::UGuildShellService()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;
}

void UGuildShellService::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGuildShellService, ShellWidth);
	DOREPLIFETIME(UGuildShellService, ShellHeight);
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
	const TValueOrError<int32, EPropertyBagResult> Width = Graph->GetGraphParameter<int32>(ShellWidthParam);
	const TValueOrError<int32, EPropertyBagResult> Height = Graph->GetGraphParameter<int32>(ShellHeightParam);
	if (Width.HasValue() && Height.HasValue())
	{
		ShellWidth = FMath::Clamp(Width.GetValue(), 1, MaxShellCells);
		ShellHeight = FMath::Clamp(Height.GetValue(), 1, MaxShellCells);
	}
}

void UGuildShellService::OnRep_ShellSize()
{
	ApplyShellSize();
}

bool UGuildShellService::SetShellSize(int32 Width, int32 Height)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return false;
	}
	if (Width < 1 || Width > MaxShellCells || Height < 1 || Height > MaxShellCells)
	{
		UE_LOG(LogTemp, Warning, TEXT("GuildShellService: size %dx%d out of range (1~%d)."), Width, Height, MaxShellCells);
		return false;
	}

	ShellWidth = Width;
	ShellHeight = Height;
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
	const TValueOrError<int32, EPropertyBagResult> CurrentWidth = Graph->GetGraphParameter<int32>(ShellWidthParam);
	const TValueOrError<int32, EPropertyBagResult> CurrentHeight = Graph->GetGraphParameter<int32>(ShellHeightParam);
	if ((PCG->bGenerated || PCG->IsGenerating())
		&& CurrentWidth.HasValue() && CurrentWidth.GetValue() == ShellWidth
		&& CurrentHeight.HasValue() && CurrentHeight.GetValue() == ShellHeight)
	{
		return;
	}

	if (Graph->SetGraphParameter<int32>(ShellWidthParam, ShellWidth) != EPropertyBagResult::Success
		|| Graph->SetGraphParameter<int32>(ShellHeightParam, ShellHeight) != EPropertyBagResult::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("GuildShellService: graph has no Width/Height parameter. Resize skipped."));
		return;
	}

	// 각 머신이 자기 쪽에서 적용하므로 복제되지 않는 로컬 호출을 쓴다.
	PCG->CleanupLocalImmediate(true);
	PCG->GenerateLocal(true);

	UE_LOG(LogTemp, Warning, TEXT("GuildShellService: shell resized to %dx%d."), ShellWidth, ShellHeight);
}
