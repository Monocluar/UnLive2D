#include "SUnLive2DMotionEditorViewport.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "UnLive2DMotionViewportClient.h"

void SUnLive2DMotionEditorViewport::Construct(const FArguments& InArgs)
{
	UnLive2DMotionBeingEdited = InArgs._UnLive2DMotionBeingEdited;

	SEditorViewport::Construct(SEditorViewport::FArguments());
}

void SUnLive2DMotionEditorViewport::BindCommands()
{
	SEditorViewport::BindCommands();
}

TSharedRef<FEditorViewportClient> SUnLive2DMotionEditorViewport::MakeEditorViewportClient()
{
	EditorViewportClient = MakeShareable(new FUnLive2DMotionViewportClient(UnLive2DMotionBeingEdited.Get()));

	return EditorViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SUnLive2DMotionEditorViewport::MakeViewportToolbar()
{
	//return SNew(SUnLive2DEditorViewportToolbar, SharedThis(this));

	return SEditorViewport::MakeViewportToolbar();
}

EVisibility SUnLive2DMotionEditorViewport::GetTransformToolbarVisibility() const
{
	return EVisibility::Visible;
}

void SUnLive2DMotionEditorViewport::OnFocusViewportToSelection()
{
	EditorViewportClient->RequestFocusOnSelection(/*bInstant=*/ false);

	SEditorViewport::OnFocusViewportToSelection();
}

TSharedRef<class SEditorViewport> SUnLive2DMotionEditorViewport::GetViewportWidget()
{
	return SharedThis(this);
}

TSharedPtr<FExtender> SUnLive2DMotionEditorViewport::GetExtenders() const
{
	TSharedPtr<FExtender> Result(MakeShareable(new FExtender));
	return Result;
}

void SUnLive2DMotionEditorViewport::OnFloatingButtonClicked()
{

}

