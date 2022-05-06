#include "UnLive2DAnimBlurprintEditorToolbar.h"
#include "UnLive2DAnimBlueprint/UnLive2DAnimationBlueprintEditor.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Toolkits/BaseToolkit.h"
#include "WorkflowOrientedApp/SModeWidget.h"
#include "ToolMenuMisc.h"
#include "ToolMenu.h"
#include "ToolMenuSection.h"

void FUnLive2DAnimBlurprintEditorToolbar::AddCompileToolbar(UToolMenu* InMenu)
{
	FToolMenuSection& Section = InMenu->AddSection("Compile");
	Section.InsertPosition = FToolMenuInsert("Asset", EToolMenuInsertType::Before);

	Section.AddDynamicEntry("CompileCommands", FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
	{

	}));
}
