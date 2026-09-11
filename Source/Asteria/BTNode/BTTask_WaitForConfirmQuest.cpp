// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_WaitForConfirmQuest.h"

UBTTask_WaitForConfirmQuest::UBTTask_WaitForConfirmQuest()
{
	NodeName = TEXT("Wait For Confirm Quest");
}

EBTNodeResult::Type UBTTask_WaitForConfirmQuest::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 무한정 대기: 스스로 끝내지 않는다.
	// 확정 이벤트가 왔을 때 외부에서 FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded)를 호출해 종료.
	return EBTNodeResult::InProgress;
}
