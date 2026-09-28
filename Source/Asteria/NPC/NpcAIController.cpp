// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NpcAIController.h"

#include "AsteriaNpc.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"

// 경로 추종을 Detour Crowd로 교체 → NPC끼리 서로 비켜 지나감(서버에서만 동작).
ANpcAIController::ANpcAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
	PrimaryActorTick.bCanEverTick = true;
}

void ANpcAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UCharacterMovementComponent* CMC = GetPawn<ACharacter>()->GetCharacterMovement();
	if (CMC != nullptr)
	{
		if (GetPathFollowingComponent()->GetStatus() != EPathFollowingStatus::Moving)
		{
			CMC->MaxWalkSpeed = BaseMaxSpeed;
			return;
		}

		FVector GoalActor = GetPathFollowingComponent()->GetPathDestination();
		if (FAISystem::IsValidLocation(GoalActor))
		{
			float Dist = FVector::Dist2D(GetPawn()->GetActorLocation(), GoalActor);

			if (Dist >= SlowdownRadius) { CMC->MaxWalkSpeed = BaseMaxSpeed; }
			else
			{
				CMC->MaxWalkSpeed = FMath::Lerp(ArrivalSpeed, BaseMaxSpeed, Dist / SlowdownRadius);
			}
		}
	}
}

void ANpcAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	UCharacterMovementComponent* CMC = GetPawn<ACharacter>()->GetCharacterMovement();
	if (CMC != nullptr)
	{
		if (BaseMaxSpeed == 0)
		{
			BaseMaxSpeed = CMC->GetMaxSpeed();
		}
	}

	if (BehaviorTree != nullptr)
	{
		RunBehaviorTree(BehaviorTree);
	}
}
