#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"
#include "SCommonEditorViewportToolbarBase.h"


class UUnLive2DAnimBase;
class FUnLive2DAnimBaseViewportClient;

class SUnLive2DAnimBaseEditorViewport : public SEditorViewport, public ICommonEditorViewportToolbarInfoProvider
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DAnimBaseEditorViewport)
		: _UnLive2DAnimBaseBeingEdited(nullptr)
	{}
		SLATE_ATTRIBUTE( UUnLive2DAnimBase*, UnLive2DAnimBaseBeingEdited )
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
	TAttribute<UUnLive2DAnimBase*> UnLive2DAnimBaseBeingEdited;

	// Viewport client
	TSharedPtr<FUnLive2DAnimBaseViewportClient> EditorViewportClient;
};