// Fill out your copyright notice in the Description page of Project Settings.


#include "CounterService.h"

#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UCounterService::UCounterService()
{
	SetIsReplicatedByDefault(true);

	PrimaryComponentTick.bCanEverTick = false;
}

void UCounterService::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCounterService, SubmittedClaimIds);
}

void UCounterService::OnRep_SubmittedClaims()
{
	OnSubmittedClaimsChanged.Broadcast();
}

bool UCounterService::SubmitClaim(int32 ClaimId)
{
	// 소유·복제 방향 불변조건: 상태 변경은 호스트만.
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return false;
	}

	// 입력 검증(경계): 유효한 ID인가 / 중복 제출은 아닌가.
	// NOTE: "실재하는 Claim인가"는 여기서 못 본다 — QuestService를 참조할지 말지(의존 방향)가 아직 미정.
	if (ClaimId == INDEX_NONE || IsClaimSubmitted(ClaimId))
	{
		return false;
	}

	SubmittedClaimIds.Add(ClaimId);

	// 서버는 OnRep이 자동 호출되지 않으므로 직접 통지.
	OnSubmittedClaimsChanged.Broadcast();

	return true;
}

bool UCounterService::IsClaimSubmitted(int32 ClaimId) const
{
	return SubmittedClaimIds.Contains(ClaimId);
}
