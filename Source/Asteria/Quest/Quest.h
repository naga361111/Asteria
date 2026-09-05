#pragma once

#include "CoreMinimal.h"
#include "Common/Rank.h"
#include "Quest.generated.h"

USTRUCT()
struct FQuest
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	int32 QuestId;
	
	UPROPERTY(VisibleAnywhere)
	bool bIsPosted;
	
	UPROPERTY(VisibleAnywhere)
	bool bIsAccepted;
	
	UPROPERTY(VisibleAnywhere)
	bool bIsCleared;
	
	UPROPERTY(VisibleAnywhere)
	ERank RecommendedRank;
};
