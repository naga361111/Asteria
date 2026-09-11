// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NpcAIController.h"

#include "AsteriaNpc.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"

ANpcAIController::ANpcAIController()
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
		UBlackboardComponent* BB = nullptr;
		if (UseBlackboard(BehaviorTree->GetBlackboardAsset(), BB))
		{
			if (AAsteriaNpc* Npc = Cast<AAsteriaNpc>(InPawn))
			{
				BB->SetValueAsObject(FName("PostQuestBoard"), Npc->PostQuestBoardActor);
				BB->SetValueAsObject(FName("GuildCounter"), Npc->GuildCounterActor);
			}
		}
		RunBehaviorTree(BehaviorTree);
	}
}
