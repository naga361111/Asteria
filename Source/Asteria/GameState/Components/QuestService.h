// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Common/Quest.h"
#include "QuestService.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnQuestPullChanged);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ASTERIA_API UQuestService : public UActorComponent
{
	GENERATED_BODY()

	int32 QuestCount = 0;

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
};
