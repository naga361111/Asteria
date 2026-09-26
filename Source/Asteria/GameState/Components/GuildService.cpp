// Fill out your copyright notice in the Description page of Project Settings.


#include "GuildService.h"

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
