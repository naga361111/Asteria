// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tools/UEdMode.h"

#include "AsteriaBuildEdMode.generated.h"

/**
 * 레벨 디자인용 배치 모드의 껍데기. Foliage/Modeling과 같은 층위의 Editor Mode다.
 *
 * 이 클래스는 모드의 수명(Enter/Exit)과 툴이 살 컨텍스트만 소유한다. 배치 로직·프리뷰·스냅은
 * 전부 여기에 등록될 InteractiveTool 쪽 책임이다 — 모드가 직접 액터를 스폰하기 시작하면
 * 툴 경계가 사라지고 모드가 만능 객체가 된다.
 *
 * 모드 등록은 UAssetEditorSubsystem이 UEdMode CDO를 순회해 처리한다. 별도 등록 코드는 없다.
 */
UCLASS()
class ASTERIAEDITOR_API UAsteriaBuildEdMode : public UEdMode
{
	GENERATED_BODY()

public:
	// 모드 식별자. 툴바 버튼·레이아웃 저장·다른 모드와의 호환성 판정이 전부 이 값을 키로 쓴다.
	static const FEditorModeID EM_AsteriaBuildEdModeId;

	UAsteriaBuildEdMode();

	virtual void Enter() override;
	virtual void CreateToolkit() override;
};
