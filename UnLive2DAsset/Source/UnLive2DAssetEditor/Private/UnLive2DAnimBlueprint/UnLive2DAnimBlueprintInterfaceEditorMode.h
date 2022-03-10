
#pragma once

#include "CoreMinimal.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"
#include "BlueprintEditorModes.h"

class FUnLive2DAnimationBlueprintEditor;

class FUnLive2DAnimBlueprintInterfaceEditorMode : public FBlueprintInterfaceApplicationMode
{
protected:
	// Set of spawnable tabs in persona mode (@TODO: Multiple lists!)
	FWorkflowAllowedTabSet TabFactories;

public:
	FUnLive2DAnimBlueprintInterfaceEditorMode(const TSharedRef<FUnLive2DAnimationBlueprintEditor>& InAnimationBlueprintEditor);

	// FApplicationMode interface
	virtual void RegisterTabFactories(TSharedPtr<FTabManager> InTabManager) override;
	// End of FApplicationMode interface

private:
	TWeakObjectPtr<class UUnLive2DAnimBlueprint> AnimBlueprintPtr;
};