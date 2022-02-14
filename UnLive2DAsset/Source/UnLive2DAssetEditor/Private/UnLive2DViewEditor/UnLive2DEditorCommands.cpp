#include "UnLive2DEditorCommands.h"

#define LOCTEXT_NAMESPACE "UnLive2DEditorCommands"

void FUnLive2DEditorCommands::RegisterCommands()
{
	UI_COMMAND(EnterViewMode, "View", "View the UnLive2D", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(EnterAnimMode, "Anim", "Anim the UnLive2D", EUserInterfaceActionType::ToggleButton, FInputChord());
}

#undef LOCTEXT_NAMESPACE