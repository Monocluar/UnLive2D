#include "AnimBlueprintGraph/UnLive2DAnimBlueprintEditorCommands.h"

#define LOCTEXT_NAMESPACE "UnLive2D"


void FUnLive2DAnimBlueprintEditorCommands::RegisterCommands()
{
	UI_COMMAND(PlayUnLive2DAnimBlueprint, "Play AnimBlueprint", "Plays the AnimBlueprint", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(PlayNode, "Play Node", "Plays Selected Node", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(StopUnLive2DAnim, "Stop", "Stops the currently playing AnimBlueprint/node", EUserInterfaceActionType::Button, FInputChord());

	UI_COMMAND(BrowserSync, "Sync to Browser", "在工程内容中选择资源", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(AddInput, "Add Input", "添加输入节点", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(DeleteInput, "Delete Input", "删除一个输入节点", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE