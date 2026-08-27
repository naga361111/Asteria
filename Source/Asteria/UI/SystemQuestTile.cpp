// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SystemQuestTile.h"

#include "QuestEntryObject.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void USystemQuestTile::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	// BP OnListItemObjectSet 이벤트가 계속 발동하도록 인터페이스 기본 구현 호출.
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	Entry = Cast<UQuestEntryObject>(ListItemObject);
	QuestIdText->SetText(FText::AsNumber(Entry->Quest.QuestId));
}

void USystemQuestTile::NativeConstruct()
{
	QuestButton->OnClicked.AddDynamic(this, &USystemQuestTile::OnConfirmButtonClicked);
}

void USystemQuestTile::OnConfirmButtonClicked()
{
	AActor* QbActor = UGameplayStatics::GetActorOfClass(GetWorld(), AQuestBoard::StaticClass());
	AQuestBoard* Qb = Cast<AQuestBoard>(QbActor);

	Qb->AddQuestsPull(Entry->Quest);
}
