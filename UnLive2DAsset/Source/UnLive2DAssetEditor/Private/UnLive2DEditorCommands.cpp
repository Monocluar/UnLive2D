#include "UnLive2DEditorCommands.h"

#define LOCTEXT_NAMESPACE "UnLive2DEditor"

void FUnLive2DEditorCommands::RegisterCommands()
{
	UI_COMMAND(SetShowGrid, "Grid", "Displays the viewport grid.", EUserInterfaceActionType::ToggleButton, FInputChord());
}

#undef LOCTEXT_NAMESPACE