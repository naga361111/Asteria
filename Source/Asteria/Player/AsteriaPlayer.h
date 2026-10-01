// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "AsteriaPlayer.generated.h"

struct FInputActionValue;
class ABuildingGrid;
class UStaticMesh;
class UMaterialInterface;
struct FBuildingGridEdge;

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

	// 선택 메시 전환 액션(1D: 마우스 휠).
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> SelectAction;

	// 휠 방향에 따라 SelectedMeshIndex를 한 칸 옮긴다. 끝을 넘으면 반대쪽 끝으로.
	void SelectMesh(const FInputActionValue& Value);

	// 잡을 수 있는 메시 목록.
	UPROPERTY(EditDefaultsOnly, Category = "Placement")
	TArray<TObjectPtr<UStaticMesh>> PlaceableMeshes;

	// 선택된 메시의 PlaceableMeshes 인덱스.
	int32 SelectedMeshIndex = 0;

	// 배치 액션(클릭).
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> PlaceAction;

	// 시선이 가리키는 변에 선택된 메시를 할당한다.
	void Place(const FInputActionValue& Value);

	// 설치 방향 뒤집기 액션.
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> FlipAction;

	// 현재 설치 방향. true면 yaw 180도로 놓는다.
	bool bPlaceFlipped = false;

	// bPlaceFlipped를 뒤집는다.
	void FlipPlacement(const FInputActionValue& Value);

	// 잡은 메시를 설치될 변 위치에 보여 주는 미리보기. 월드 트랜스폼을 직접 지정한다.
	UPROPERTY(EditDefaultsOnly, Category = "Placement")
	TObjectPtr<UStaticMeshComponent> PreviewComp;

	// 미리보기 메시의 앞(+Y) 바닥에 화살표를 깔아 앞 방향을 보여주는 데칼. PreviewComp의 자식이라 위치·뒤집힘 회전을 따른다. 재질은 블루프린트에서 지정.
	UPROPERTY(EditDefaultsOnly, Category = "Placement")
	TObjectPtr<UDecalComponent> PreviewDecalComp;

	// 미리보기의 모든 재질 슬롯에 덮어쓸 반투명 재질.
	UPROPERTY(EditDefaultsOnly, Category = "Placement")
	TObjectPtr<UMaterialInterface> PreviewMaterial;

	// Edge가 있고 선택된 메시가 유효하면 그 변 위치에 미리보기를 띄우고, 아니면 숨긴다. Tick()이 매 프레임 호출.
	void UpdatePreview(const FBuildingGridEdge* Edge);

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

	// 시선이 가리키는 변을 찾을 격자. 레벨의 첫 번째 ABuildingGrid. BeginPlay()가 설정.
	TWeakObjectPtr<ABuildingGrid> Grid;

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

	// 1인칭 시선 광선. 뷰포트 중앙 픽셀(조준점)을 월드로 역투영한다.
	// 컨트롤러가 없거나 역투영 실패(서버, 초기화 전)면 false.
	bool GetViewRay(FRay& OutRay) const;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
