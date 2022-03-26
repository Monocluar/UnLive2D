
#pragma once

#include "CoreMinimal.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"
#include "BlueprintEditorModes.h"

class UUnLive2DAnimBlueprint;
class FPreviewScene;

class FUnLive2DAnimBlueprintEditorMode : public FBlueprintEditorApplicationMode
{
protected:
	// Set of spawnable tabs in persona mode (@TODO: Multiple lists!)
	FWorkflowAllowedTabSet TabFactories;

public:
	FUnLive2DAnimBlueprintEditorMode(const TSharedRef<FUnLive2DAnimationBlueprintEditor>& InAnimationBlueprintEditor);

	// FApplicationMode interface
	virtual void RegisterTabFactories(TSharedPtr<FTabManager> InTabManager) override;
	virtual void PostActivateMode() override;
	// End of FApplicationMode interface


private:
	//TWeakPtr<FPreviewScene> PreviewScenePtr;

	TWeakObjectPtr<UUnLive2DAnimBlueprint> AnimBlueprintPtr;

};