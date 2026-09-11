// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_WaitForConfirmQuest.h"

#include "AIController.h"
#include "NPC/AsteriaNpc.h"
#include "GameState/AsteriaGameState.h"
#include "GameState/Components/QuestService.h"
#include "GameState/Components/CounterService.h"

// 이 태스크의 실행별 상태. 노드는 트리를 쓰는 모든 AI가 공유하는 단일 인스턴스라
// 멤버 변수에 두면 서로 덮어쓴다 → NodeMemory에 담는다.
struct FBTWaitForConfirmQuestMemory
{
	TWeakObjectPtr<UQuestService> QuestService;
	FDelegateHandle ConfirmedHandle;
	int32 WaitingClaimId = INDEX_NONE;
};

UBTTask_WaitForConfirmQuest::UBTTask_WaitForConfirmQuest()
{
	NodeName = TEXT("Wait For Confirm Quest");
}

EBTNodeResult::Type UBTTask_WaitForConfirmQuest::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// BT는 서버에서만 도는 전제. 월드/NpcId는 OwnerComp의 AI 오너·폰에서 얻는다.
	// (노드의 GetWorld()는 Outer가 월드가 아니라 신뢰 불가)
	AAIController* AICon = OwnerComp.GetAIOwner();
	AAsteriaNpc* Npc = AICon ? Cast<AAsteriaNpc>(AICon->GetPawn()) : nullptr;
	if (Npc == nullptr) return EBTNodeResult::Failed;

	AAsteriaGameState* GS = Npc->GetWorld()->GetGameState<AAsteriaGameState>();
	if (GS == nullptr) return EBTNodeResult::Failed;

	UQuestService* Service = GS->QuestService;
	if (Service == nullptr) return EBTNodeResult::Failed;

	UCounterService* Counter = GS->CounterService;
	if (Counter == nullptr) return EBTNodeResult::Failed;

	// 내가 기다릴 Claim = 파티에 내 NpcId가 든 Claim. (Claims는 public이라 새 접근자 없이 되짚음)
	// TODO: SelectQuest가 ClaimId를 넘겨주는 경계가 서면 그 값을 받아 이 순회를 대체.
	int32 WaitingClaimId = INDEX_NONE;
	for (const FQuestClaim& Claim : Service->Claims)
	{
		if (Claim.Party.Contains(Npc->NpcId))
		{
			WaitingClaimId = Claim.ClaimId;
			break;
		}
	}
	if (WaitingClaimId == INDEX_NONE) return EBTNodeResult::Failed; // 집은 게 없으면 기다릴 대상도 없음

	FBTWaitForConfirmQuestMemory* Mem = CastInstanceNodeMemory<FBTWaitForConfirmQuestMemory>(NodeMemory);
	Mem->QuestService = Service;
	Mem->WaitingClaimId = WaitingClaimId;

	// 밖에서 깨우고(QuestService가 컨펌 시 OnClaimConfirmed.Broadcast) → 안에서 끝낸다(이 람다가 FinishLatentTask 호출).
	// 브로드캐스트는 어느 Claim이든 오므로 내 ClaimId만 필터.
	Mem->ConfirmedHandle = Service->OnClaimConfirmed.AddLambda(
		[this, &OwnerComp, Mem](int32 ConfirmedClaimId)
		{
			if (ConfirmedClaimId == Mem->WaitingClaimId)
			{
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			}
		});

	// 구독을 먼저, 제출은 그 다음 — 순서가 바뀌면 제출 직후의 컨펌을 놓친다.
	// 실패하면 기다릴 이유가 없다 → Failed. (구독 해제는 OnTaskFinished가 책임)
	if (!Counter->SubmitClaim(WaitingClaimId))
	{
		UE_LOG(LogTemp, Warning, TEXT("SubmitClaim failed. Claim:%d"), WaitingClaimId)
		return EBTNodeResult::Failed;
	}

	UE_LOG(LogTemp, Warning, TEXT("Wait For Claim:%d Confirm"), WaitingClaimId)
	return EBTNodeResult::InProgress;
}

void UBTTask_WaitForConfirmQuest::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	// 성공/실패/Abort 어느 경로로 끝나든 여기로 온다 → 구독을 반드시 해제(댕글링 방지).
	FBTWaitForConfirmQuestMemory* Mem = CastInstanceNodeMemory<FBTWaitForConfirmQuestMemory>(NodeMemory);
	if (Mem->QuestService.IsValid() && Mem->ConfirmedHandle.IsValid())
	{
		Mem->QuestService->OnClaimConfirmed.Remove(Mem->ConfirmedHandle);
	}
	Mem->ConfirmedHandle.Reset();
	Mem->QuestService.Reset();
	Mem->WaitingClaimId = INDEX_NONE;

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

uint16 UBTTask_WaitForConfirmQuest::GetInstanceMemorySize() const
{
	return sizeof(FBTWaitForConfirmQuestMemory);
}

void UBTTask_WaitForConfirmQuest::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
	// NodeMemory는 raw. 비POD 멤버(TWeakObjectPtr/FDelegateHandle)를 위해 placement new로 초기화.
	// (조기 Fail 리턴 경로에서도 OnTaskFinished가 유효한 핸들 상태를 보게 하는 안전장치)
	new (NodeMemory) FBTWaitForConfirmQuestMemory();
}

void UBTTask_WaitForConfirmQuest::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const
{
	CastInstanceNodeMemory<FBTWaitForConfirmQuestMemory>(NodeMemory)->~FBTWaitForConfirmQuestMemory();
}
