// Fill out your copyright notice in the Description page of Project Settings.

#include "AsteriaEditor.h"

#define LOCTEXT_NAMESPACE "FAsteriaEditorModule"

void FAsteriaEditorModule::StartupModule()
{
	// UEdMode는 CDO 순회로 자동 등록되므로 여기서 모드를 등록할 필요는 없다.
	// 툴 팔레트용 TCommands가 생기면 등록/해제가 붙을 자리.
}

void FAsteriaEditorModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAsteriaEditorModule, AsteriaEditor)
