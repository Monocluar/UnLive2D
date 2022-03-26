#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"
#include "SCommonEditorViewportToolbarBase.h"


class UUnLive2DAnimBlueprint;
class FUnLive2DAnimBlueprintViewportClient;

class SUnLive2DAnimBlueprintEditorViewport : public SEditorViewport, public ICommonEditorViewportToolbarInfoProvider
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DAnimBlueprintEditorViewport)
		: _UnLive2DAnimBlueprintEdited(nullptr)
		{}
		SLATE_ATTRIBUTE(UUnLive2DAnimBlueprint*, UnLive2DAnimBlueprintEdited)
	SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs);

public:
	// SEditorViewport interface
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual EVisibility GetTransformToolbarVisibility() const override;
	virtual void OnFocusViewportToSelection() override;
	// End of SEditorViewport interface

public:
	// ICommonEditorViewportToolbarInfoProvider interface
	virtual TSharedRef<class SEditorViewport> GetViewportWidget() override;
	virtual TSharedPtr<FExtender> GetExtenders() const override;
	virtual void OnFloatingButtonClicked() override;
	// End of ICommonEditorViewportToolbarInfoProvider interface

private:
	TAttribute<UUnLive2DAnimBlueprint*> UnLive2DAnimBlueprintEdited;

	// Viewport client
	TSharedPtr<FUnLive2DAnimBlueprintViewportClient> EditorViewportClient;
};