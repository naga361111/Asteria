// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GuildShellService.generated.h"

/**
 * 길드 건물 외곽 크기(가운데 칸을 뺀 한쪽 칸 수, 칸 수 = 2N+1)를 소유한다. 쓰기는 서버 권위, 클라는 복제된 값을 받아 각자 PCG 그래프 파라미터를 바꿔 다시 만든다.
 * 레벨에 저장된 초기 크기는 GuildShellVolume 디테일 패널의 그래프 파라미터(HalfWidth·HalfHeight)가 정한다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ASTERIA_API UGuildShellService : public UActorComponent
{
	GENERATED_BODY()

public:
	UGuildShellService();

	// 서버에서 레벨의 PCG 파라미터(디테일 패널 값)를 읽어 초기 한쪽 칸 수로 삼는다. 클라는 복제로 받는다.
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 외곽 가로 한쪽 칸 수(한 칸 300, 칸 수 = 2N+1). 쓰기는 서버 권위(SetShellSize).
	// 기본값 -1 = 아직 정해지지 않음. 기본값과 같은 값은 복제되지 않으므로, 실제 크기(0 이상)는 항상 클라로 복제돼 OnRep이 불린다.
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_ShellSize, Category="GuildShell")
	int32 ShellHalfWidth = -1;

	// 외곽 세로 한쪽 칸 수(한 칸 300, 칸 수 = 2N+1). 쓰기는 서버 권위(SetShellSize). 기본값 -1의 이유는 ShellHalfWidth와 같다.
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_ShellSize, Category="GuildShell")
	int32 ShellHalfHeight = -1;

	// 가로·세로가 같은 OnRep을 공유한다. 둘 다 바뀌면 두 번 불리지만 ApplyShellSize가 두 번째를 건너뛴다.
	UFUNCTION()
	void OnRep_ShellSize();

	// 외곽 한쪽 칸 수를 바꾸고 PCG를 다시 만든다. 실패(권위 없음/0~BuildingLayout::MaxHalfCells 범위 밖) 시 false.
	bool SetShellSize(int32 HalfWidth, int32 HalfHeight);

private:
	// 태그 GuildShellVolume 액터의 PCG 컴포넌트. 없으면 null.
	class UPCGComponent* FindShellPCG() const;

	// PCG 그래프 파라미터 HalfWidth·HalfHeight를 현재 한쪽 칸 수로 바꾸고 PCG를 강제 재생성한다. 이미 같은 크기면 건너뛴다.
	void ApplyShellSize();
};
