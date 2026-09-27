// Fill out your copyright notice in the Description page of Project Settings.


#include "GuildService.h"

#include "Data/GuildReputationData.h"
#include "Net/UnrealNetwork.h"

UGuildService::UGuildService()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;
}

void UGuildService::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGuildService, GuildFunds);
	DOREPLIFETIME(UGuildService, GuildRank);
	DOREPLIFETIME(UGuildService, GuildReputation);
}

void UGuildService::OnRep_GuildFunds()
{
	OnGuildFundsChanged.Broadcast();
}

void UGuildService::OnRep_GuildReputation()
{
	OnGuildReputationChanged.Broadcast();
}

bool UGuildService::AddGuildFunds(int32 Amount)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || Amount <= 0)
	{
		return false;
	}

	GuildFunds += Amount;
	// 서버(호스트)에선 OnRep이 불리지 않으므로 직접 통지한다.
	OnGuildFundsChanged.Broadcast();
	return true;
}

bool UGuildService::AddGuildReputation(int32 Amount)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || Amount <= 0)
	{
		return false;
	}

	GuildReputation += Amount;
	UE_LOG(LogTemp, Warning, TEXT("GuildService: reputation +%d (total %d)"), Amount, GuildReputation);

	if (!ReputationData)
	{
		UE_LOG(LogTemp, Warning, TEXT("GuildService: no ReputationData. Rank up skipped."));
		// 서버(호스트)에선 OnRep이 불리지 않으므로 직접 통지한다.
		OnGuildReputationChanged.Broadcast();
		return true;
	}

	while (GuildRank < ERank::S)
	{
		const ERank NextRank = static_cast<ERank>(static_cast<uint8>(GuildRank) + 1);
		const int32* Required = ReputationData->ReputationToReachRank.Find(NextRank);
		if (!Required)
		{
			UE_LOG(LogTemp, Warning, TEXT("GuildService: no reputation requirement for rank %d. Rank up skipped."), static_cast<int32>(NextRank));
			break;
		}
		if (GuildReputation < *Required)
		{
			break;
		}
		GuildRank = NextRank;
		UE_LOG(LogTemp, Warning, TEXT("GuildService: rank up to %s (reputation %d / required %d)"),
			*StaticEnum<ERank>()->GetDisplayNameTextByValue(static_cast<int64>(GuildRank)).ToString(), GuildReputation, *Required);
	}

	// 서버(호스트)에선 OnRep이 불리지 않으므로 직접 통지한다.
	OnGuildReputationChanged.Broadcast();
	return true;
}

float UGuildService::GetReputationProgress() const
{
	if (GuildRank >= ERank::S)
	{
		return 1.f;
	}
	if (!ReputationData)
	{
		return 0.f;
	}

	const ERank NextRank = static_cast<ERank>(static_cast<uint8>(GuildRank) + 1);
	const int32* CurrentRequired = ReputationData->ReputationToReachRank.Find(GuildRank);
	const int32* NextRequired = ReputationData->ReputationToReachRank.Find(NextRank);
	if (!CurrentRequired || !NextRequired)
	{
		return 0.f;
	}

	const int32 Span = *NextRequired - *CurrentRequired;
	if (Span <= 0)
	{
		return 0.f;
	}

	return FMath::Clamp(static_cast<float>(GuildReputation - *CurrentRequired) / Span, 0.f, 1.f);
}
