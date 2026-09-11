// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CounterService.generated.h"

// 창구 제출 목록이 바뀌었음을 알리는 신호. (서버는 직접 Broadcast, 클라는 OnRep이 Broadcast)
DECLARE_MULTICAST_DELEGATE(FOnSubmittedClaimsChanged);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ASTERIA_API UCounterService : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCounterService();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- 창구 제출함: NPC가 컨펌을 요구하며 올린 Claim들 ---
	// Claim 본체(FQuestClaim)는 QuestService가 소유한다. 여기는 ID 참조만 들고 있는 목록이다.
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_SubmittedClaims, Category="Counter")
	TArray<int32> SubmittedClaimIds;

	UFUNCTION()
	void OnRep_SubmittedClaims();

	FOnSubmittedClaimsChanged OnSubmittedClaimsChanged;

	// 서버 권위: NPC가 컨펌 요구 퀘스트를 창구에 올린다.
	// 실패(권위 없음/잘못된 ID/이미 제출됨) 시 false.
	bool SubmitClaim(int32 ClaimId);

	bool IsClaimSubmitted(int32 ClaimId) const;

	// TODO: 컨펌·반려로 제출함에서 빼는 경계(RemoveSubmittedClaim 등)는 아직 없다.
	// 누가 빼는지(QuestService의 ConfirmClaim이냐, 창구 UI 입력이냐)가 정해지면 그때 연다.
};
