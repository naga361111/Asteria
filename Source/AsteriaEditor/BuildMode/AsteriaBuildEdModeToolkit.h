// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/BaseToolkit.h"

/**
 * 빌드 모드의 에디터 패널. 팔레트(피스 선택 UI)와 활성 툴의 프로퍼티가 여기에 뜬다.
 * 지금은 기본 팔레트 하나만 선언한 빈 껍데기다.
 */
class FAsteriaBuildEdModeToolkit : public FModeToolkit
{
public:
	// FModeToolkit interface
	virtual void GetToolPaletteNames(TArray<FName>& PaletteNames) const override;

	// IToolkit interface
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
};
