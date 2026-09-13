// Fill out your copyright notice in the Description page of Project Settings.

#include "BuildMode/AsteriaBuildEdModeToolkit.h"

#define LOCTEXT_NAMESPACE "AsteriaBuildEdModeToolkit"

void FAsteriaBuildEdModeToolkit::GetToolPaletteNames(TArray<FName>& PaletteNames) const
{
	PaletteNames.Add(NAME_Default);
}

FName FAsteriaBuildEdModeToolkit::GetToolkitFName() const
{
	return FName("AsteriaBuildEdMode");
}

FText FAsteriaBuildEdModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("DisplayName", "Asteria Build");
}

#undef LOCTEXT_NAMESPACE
