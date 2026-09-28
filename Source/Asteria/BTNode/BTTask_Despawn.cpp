// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Despawn.h"

#include "AIController.h"

UBTTask_Despawn::UBTTask_Despawn()
{
	NodeName = TEXT("Despawn");

	// 실행별 상태가 없다 → NodeMemory·노드 인스턴스화를 쓰지 않는다(공유 인스턴스 그대로).
}

EBTNodeResult::Type UBTTask_Despawn::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
	if (Pawn == nullptr) return EBTNodeResult::Failed;

	// 제거는 서버에서만 — 클라는 리플리케이션으로 사라진다.
	if (!Pawn->HasAuthority()) return EBTNodeResult::Failed;

	// 실행 중인 BT의 주인을 태스크 안에서 바로 Destroy하지 않는다 → 수명으로 다음 틱 이후에 제거.
	// (0은 "무한 수명"이라 반드시 0보다 커야 한다)
	Pawn->SetLifeSpan(0.1f);

	UE_LOG(LogTemp, Warning, TEXT("Despawn scheduled: %s"), *Pawn->GetName())

	// Succeeded로 끝내면 시퀀스가 처음(SelectQuest)부터 다시 돌아 제거 전에 퀘스트를 또 집을 수 있다 → 제거될 때까지 대기.
	return EBTNodeResult::InProgress;
}
