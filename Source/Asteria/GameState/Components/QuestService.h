// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Common/Quest.h"
#include "QuestService.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnQuestPullChanged);
DECLARE_MULTICAST_DELEGATE(FOnClaimsChanged);
// 특정 Claim이 Pending→Accepted로 확정됐음을 알리는 서버 로컬 신호. 인자는 확정된 ClaimId.
// (WaitForConfirm BT 태스크가 자기 ClaimId만 필터해 FinishLatentTask 호출용. 복제 아님)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnClaimConfirmed, int32 /*ClaimId*/);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ASTERIA_API UQuestService : public UActorComponent
{
	GENERATED_BODY()

	int32 QuestCount = 0;
	int32 ClaimCount = 0;

public:
	// Sets default values for this component's properties
	UQuestService();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_QuestPull, Category="Quest")
	TArray<FQuest> QuestPull;

	UFUNCTION()
	void OnRep_QuestPull();

	// QuestPull이 바뀌면 이걸 Broadcast (호출자 책임)
	FOnQuestPullChanged OnQuestPullChanged;

	// --- 소유권(Claim): 퀘스트-NPC 소유의 단일 진실원 ---

	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_Claims, Category="Quest")
	TArray<FQuestClaim> Claims;

	UFUNCTION()
	void OnRep_Claims();

	FOnClaimsChanged OnClaimsChanged;
	
	FOnClaimConfirmed OnClaimConfirmed;

	// 서버 권위: 퀘스트를 파티가 집는다(보드 선택 = 소유권 획득). Select+Assign을 하나로.
	// 성공 시 ClaimId, 실패(권위 없음/미존재/이미 집힘/빈 파티) 시 INDEX_NONE.
	int32 ClaimQuest(int32 QuestId, const TArray<int32>& Party);

	// 가용성 파생의 단일 원천. 클라에서도 복제된 Claims를 읽어 동일 판정.
	bool IsQuestClaimed(int32 QuestId) const;

	// 아직 안 집힌 퀘스트의 QuestId 하나. 없으면 INDEX_NONE. (BT의 "무엇을 집을까" 선택용)
	int32 FindAvailableQuestId() const;
};
