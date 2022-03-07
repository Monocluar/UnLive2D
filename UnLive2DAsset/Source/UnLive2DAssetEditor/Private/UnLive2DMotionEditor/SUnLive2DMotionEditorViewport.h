#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"
#include "SCommonEditorViewportToolbarBase.h"


class UUnLive2DMotion;
class FUnLive2DMotionViewportClient;

class SUnLive2DMotionEditorViewport : public SEditorViewport, public ICommonEditorViewportToolbarInfoProvider
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DMotionEditorViewport)
		: _UnLive2DMotionBeingEdited(nullptr)
	{}
		SLATE_ATTRIBUTE( UUnLive2DMotion*, UnLive2DMotionBeingEdited )
	SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs);

public:
	// SEditorViewport interface
	virtual void BindCommands() override;
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;
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
	TAttribute<UUnLive2DMotion*> UnLive2DMotionBeingEdited;

	// Viewport client
	TSharedPtr<FUnLive2DMotionViewportClient> EditorViewportClient;
};