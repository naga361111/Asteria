// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "AsteriaPlayer.generated.h"

struct FInputActionValue;

UCLASS()
class ASTERIA_API AAsteriaPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAsteriaPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Input mapping context applied to this player on possession.
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;

	// Move action (2D: forward/back + right/left).
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	// Aim/Look action (2D: yaw + pitch).
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact(const FInputActionValue& Value);
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UCameraComponent> CameraComp;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UBoxComponent> BoxComp;
	
	// OnComponentBeginOverlap 델리게이트 시그니처와 1:1 대응. UFUNCTION() 필수(동적 델리게이트라 리플렉션 필요).
	UFUNCTION()
	void OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// OnComponentEndOverlap 델리게이트 시그니처. Begin과 달리 bFromSweep/SweepResult 인자가 없다.
	UFUNCTION()
	void OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	TWeakObjectPtr<AActor> OverlappedActor;

	// UI 커서 상태는 로컬 클라 전용. 월드 액터가 아니라 로컬 플레이어가 소유한다.
	bool bUIInputMode = false;

public:
	// 커서/입력 모드 전환. true면 GameAndUI + 커서 표시, false면 GameOnly + 커서 숨김.
	void SetUIInputMode(bool bEnable);

	// 현재 상태를 뒤집는다. 상호작용 토글용.
	void ToggleUIInputMode() { SetUIInputMode(!bUIInputMode); }

	// 창구 수락 입력의 클라→서버 경계.
	// 상태 소유자는 GameState의 CounterService지만 GameState는 클라가 소유한 액터가 아니라
	// 그 위의 Server RPC는 라우팅되지 않고 버려진다. 그래서 소유 액터인 폰이 대신 받아 넘긴다.
	UFUNCTION(Server, Reliable)
	void Server_AcceptQuestAssignment(int32 AssignmentId);

	// 창구 정산 확정 입력의 클라→서버 경계. 수락과 같은 이유로 폰이 대신 받는다.
	UFUNCTION(Server, Reliable)
	void Server_SettleQuestAssignment(int32 AssignmentId);

	bool IsUIInputMode() const { return bUIInputMode; }

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
