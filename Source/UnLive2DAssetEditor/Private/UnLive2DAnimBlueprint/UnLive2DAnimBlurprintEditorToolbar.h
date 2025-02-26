
#pragma once

#include "CoreMinimal.h"

class FUnLive2DAnimationBlueprintEditor;

class FUnLive2DAnimBlurprintEditorToolbar :  public TSharedFromThis<FUnLive2DAnimBlurprintEditorToolbar>
{
public:
	FUnLive2DAnimBlurprintEditorToolbar(TSharedPtr<FUnLive2DAnimationBlueprintEditor> InUnLive2DAnimBlueprintEditor)
		: UnLive2DAnimationBlueprintEditor(InUnLive2DAnimBlueprintEditor) {}

public:

	void AddCompileToolbar(UToolMenu* InMenu);


protected:

	TWeakPtr<FUnLive2DAnimationBlueprintEditor> UnLive2DAnimationBlueprintEditor;
};