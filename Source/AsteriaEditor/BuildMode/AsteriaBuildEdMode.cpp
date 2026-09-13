// Fill out your copyright notice in the Description page of Project Settings.

#include "BuildMode/AsteriaBuildEdMode.h"

#include "BuildMode/AsteriaBuildEdModeToolkit.h"
#include "BuildMode/AsteriaPlacePieceTool.h"

#include "InteractiveToolManager.h"
#include "Styling/AppStyle.h"
#include "Tools/EdModeInteractiveToolsContext.h"

#define LOCTEXT_NAMESPACE "AsteriaBuildEdMode"

const FEditorModeID UAsteriaBuildEdMode::EM_AsteriaBuildEdModeId = TEXT("EM_AsteriaBuild");

namespace
{
	const FString PlacePieceToolIdentifier = TEXT("AsteriaPlacePieceTool");
}

UAsteriaBuildEdMode::UAsteriaBuildEdMode()
{
	Info = FEditorModeInfo(
		EM_AsteriaBuildEdModeId,
		LOCTEXT("AsteriaBuildEdModeName", "Asteria Build"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.OpenPlaceActors"),
		/*InVisibility=*/true);
}

void UAsteriaBuildEdMode::Enter()
{
	UEdMode::Enter();

	// 툴이 하나뿐이라 팔레트 버튼(FUICommandInfo)을 만들지 않고 직접 등록한 뒤 바로 띄운다.
	// 툴이 둘 이상이 되는 순간 TCommands + UEdMode::RegisterTool로 갈아탄다.
	GetToolManager()->RegisterToolType(PlacePieceToolIdentifier, NewObject<UAsteriaPlacePieceToolBuilder>(this));
	GetInteractiveToolsContext()->StartTool(PlacePieceToolIdentifier);
}

void UAsteriaBuildEdMode::CreateToolkit()
{
	Toolkit = MakeShared<FAsteriaBuildEdModeToolkit>();
}

#undef LOCTEXT_NAMESPACE
