// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NpcAIController.h"

#include "AsteriaNpc.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"

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

	TArray<AActor*> PostQuestBoardActor;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("PostQuestBoard"), PostQuestBoardActor);

	TArray<AActor*> NpcHomeActor;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("NpcHome"), NpcHomeActor);

	if (BehaviorTree != nullptr)
	{
		RunBehaviorTree(BehaviorTree);

		if (PostQuestBoardActor.Num() != 0)
		{
			GetBlackboardComponent()->SetValueAsObject(FName("PostQuestBoard"), PostQuestBoardActor[0]);
		}

		if (NpcHomeActor.Num() != 0)
		{
			GetBlackboardComponent()->SetValueAsObject(FName("NpcHome"), NpcHomeActor[0]);
		}
	}
}
