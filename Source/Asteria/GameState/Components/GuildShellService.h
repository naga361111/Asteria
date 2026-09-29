// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GuildShellService.generated.h"

/**
 * 길드 건물 외곽 크기(칸 수)를 소유한다. 쓰기는 서버 권위, 클라는 복제된 값을 받아 각자 외곽선과 PCG를 다시 만든다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ASTERIA_API UGuildShellService : public UActorComponent
{
	GENERATED_BODY()

public:
	UGuildShellService();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 외곽 가로 칸 수(한 칸 300). 쓰기는 서버 권위(SetShellSize).
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_ShellSize, Category="GuildShell")
	int32 ShellWidth = 8;

	// 외곽 세로 칸 수(한 칸 300). 쓰기는 서버 권위(SetShellSize).
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_ShellSize, Category="GuildShell")
	int32 ShellHeight = 8;

	// 가로·세로는 함께 바뀌므로 같은 OnRep을 공유한다.
	UFUNCTION()
	void OnRep_ShellSize();

	// 외곽 칸 수를 바꾸고 외곽선·PCG를 다시 만든다. 실패(권위 없음/1~32 범위 밖) 시 false.
	bool SetShellSize(int32 Width, int32 Height);

private:
	// 태그 GuildShell 액터의 스플라인을 현재 칸 수의 닫힌 사각형으로 바꾸고, 태그 GuildShellVolume 액터의 PCG를 강제 재생성한다.
	void ApplyShellSize();
};
