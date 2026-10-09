#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AsteriaModSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAsteriaMod, Log, All);

// 모드 진입점. <프로젝트>/Mods/<ModName>/ 에 놓인 콘텐츠 플러그인은 엔진이 Mod 타입으로 자동 마운트한다.
// 모드가 자기 콘텐츠 루트에 ModEntry 블루프린트(Actor)를 두면, 게임 월드 시작 시 서버가 하나씩 스폰한다.
// 모드 로직은 전부 그 액터의 BeginPlay에서 시작한다. ModEntry가 없는 모드는 에셋 덮어쓰기 전용으로 본다.
UCLASS()
class ASTERIA_API UAsteriaModSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

protected:
	// 에디터 월드(레벨 편집 중)에서는 모드를 돌리지 않는다.
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
};
