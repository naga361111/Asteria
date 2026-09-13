// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * 에디터 전용 오서링 툴이 사는 모듈. 런타임(Asteria) 모듈은 여기를 알지 못한다 —
 * 의존 방향은 AsteriaEditor → Asteria 한 방향뿐이다.
 *
 * 이 모듈의 산출물은 에디터 타임 스태틱 지오메트리라 프로젝트의 서버 권위·복제 계약 밖에 있다.
 */
class FAsteriaEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
