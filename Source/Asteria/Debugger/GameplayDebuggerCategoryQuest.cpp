#include "GameplayDebuggerCategoryQuest.h"

#if WITH_GAMEPLAY_DEBUGGER

#include "GameplayDebuggerTypes.h"
#include "GameFramework/PlayerController.h"
#include "GameState/AsteriaGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Quest/QuestBoard.h"

FGameplayDebuggerCategoryQuest::FGameplayDebuggerCategoryQuest()
{
}

TSharedRef<FGameplayDebuggerCategory> FGameplayDebuggerCategoryQuest::MakeInstance()
{
	return MakeShareable(new FGameplayDebuggerCategoryQuest());
}

void FGameplayDebuggerCategoryQuest::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
}

void FGameplayDebuggerCategoryQuest::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	// ponytail: local read of GameState, no data-pack replication — fine for standalone/PIE.
	// Add SetDataPackReplication + FRepData when quest debug must work on a network client.
	const UWorld* World = OwnerPC ? OwnerPC->GetWorld() : nullptr;
	const AAsteriaGameState* GameState = World ? World->GetGameState<AAsteriaGameState>() : nullptr;
	if (GameState == nullptr)
	{
		CanvasContext.Printf(TEXT("{red}No AAsteriaGameState"));
		return;
	}

	CanvasContext.Printf(TEXT("{yellow}System quest pulls: {white}%d"), GameState->SystemQuestPulls.Num());
	
	AActor* QbActor = UGameplayStatics::GetActorOfClass(World, AQuestBoard::StaticClass());
	AQuestBoard* Qb = Cast<AQuestBoard>(QbActor);
	if (Qb == nullptr)
	{
		CanvasContext.Printf(TEXT("{red}No AQuestBoard"));
		return;
	}
	
	CanvasContext.Printf(TEXT("{yellow}quest board pulls: {white}%d"), Qb->QuestsPull.Num());
}

#endif // WITH_GAMEPLAY_DEBUGGER
