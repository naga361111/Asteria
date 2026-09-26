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

bool UGuildService::AddGuildFunds(int32 Amount)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || Amount <= 0)
	{
		return false;
	}

	GuildFunds += Amount;
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

	return true;
}
