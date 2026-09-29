// Fill out your copyright notice in the Description page of Project Settings.


#include "GuildShellService.h"

#include "Components/SplineComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PCGComponent.h"

namespace
{
	// Hearthvale 부품 한 칸 크기.
	constexpr float ShellCellSize = 300.f;
	// GuildShellVolume 범위(0~9600)와 맞춘 상한.
	constexpr int32 MaxShellCells = 32;
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

void UGuildShellService::ApplyShellSize()
{
	TArray<AActor*> ShellActors;
	UGameplayStatics::GetAllActorsWithTag(this, TEXT("GuildShell"), ShellActors);
	USplineComponent* Outline = ShellActors.Num() > 0 ? ShellActors[0]->FindComponentByClass<USplineComponent>() : nullptr;

	TArray<AActor*> VolumeActors;
	UGameplayStatics::GetAllActorsWithTag(this, TEXT("GuildShellVolume"), VolumeActors);
	UPCGComponent* PCG = VolumeActors.Num() > 0 ? VolumeActors[0]->FindComponentByClass<UPCGComponent>() : nullptr;

	if (!Outline || !PCG)
	{
		UE_LOG(LogTemp, Warning, TEXT("GuildShellService: outline spline or PCG component not found. Resize skipped."));
		return;
	}

	const float X = ShellWidth * ShellCellSize;
	const float Y = ShellHeight * ShellCellSize;
	const FVector Corners[] = { FVector(0.f, 0.f, 0.f), FVector(X, 0.f, 0.f), FVector(X, Y, 0.f), FVector(0.f, Y, 0.f) };

	Outline->ClearSplinePoints(false);
	for (int32 i = 0; i < UE_ARRAY_COUNT(Corners); ++i)
	{
		Outline->AddSplinePoint(Corners[i], ESplineCoordinateSpace::Local, false);
		Outline->SetSplinePointType(i, ESplinePointType::Linear, false);
	}
	Outline->SetClosedLoop(true, false);
	Outline->UpdateSpline();

	// 각 머신이 자기 쪽에서 적용하므로 복제되지 않는 로컬 호출을 쓴다.
	PCG->CleanupLocalImmediate(true);
	PCG->GenerateLocal(true);

	UE_LOG(LogTemp, Warning, TEXT("GuildShellService: shell resized to %dx%d."), ShellWidth, ShellHeight);
}
