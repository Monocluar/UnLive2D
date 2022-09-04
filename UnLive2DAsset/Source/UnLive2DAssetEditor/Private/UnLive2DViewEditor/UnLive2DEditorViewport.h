#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"
#include "SCommonEditorViewportToolbarBase.h"

class UUnLive2D;
class FUnLive2DViewViewportClient;
class FUnLive2DViewEditor;

class SUnLive2DEditorViewport : public SEditorViewport, public ICommonEditorViewportToolbarInfoProvider
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DEditorViewport)
		: _UnLive2DBeingEdited(nullptr)
	{}
		SLATE_ATTRIBUTE( UUnLive2D*, UnLive2DBeingEdited )
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
	TAttribute<UUnLive2D*> UnLive2DBeingEdited;

	// Viewport client
	TSharedPtr<FUnLive2DViewViewportClient> EditorViewportClient;

};