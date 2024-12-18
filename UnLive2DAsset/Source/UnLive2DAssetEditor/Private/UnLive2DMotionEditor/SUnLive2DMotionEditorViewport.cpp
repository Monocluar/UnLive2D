#include "SUnLive2DMotionEditorViewport.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "UnLive2DMotionViewportClient.h"

void SUnLive2DAnimBaseEditorViewport::Construct(const FArguments& InArgs)
{
	UnLive2DAnimBaseBeingEdited = InArgs._UnLive2DAnimBaseBeingEdited;

	SEditorViewport::Construct(SEditorViewport::FArguments());
}

void SUnLive2DAnimBaseEditorViewport::BindCommands()
{
	SEditorViewport::BindCommands();
}

TSharedRef<FEditorViewportClient> SUnLive2DAnimBaseEditorViewport::MakeEditorViewportClient()
{
	UUnLive2DAnimBase* UnLive2DAnimBase = UnLive2DAnimBaseBeingEdited.Get();
	EditorViewportClient = MakeShared<FUnLive2DAnimBaseViewportClient>(UnLive2DAnimBase);

	return EditorViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SUnLive2DAnimBaseEditorViewport::MakeViewportToolbar()
{
	//return SNew(SUnLive2DEditorViewportToolbar, SharedThis(this));

	return SEditorViewport::MakeViewportToolbar();
}

EVisibility SUnLive2DAnimBaseEditorViewport::GetTransformToolbarVisibility() const
{
	return EVisibility::Visible;
}

void SUnLive2DAnimBaseEditorViewport::OnFocusViewportToSelection()
{
	EditorViewportClient->RequestFocusOnSelection(/*bInstant=*/ false);

	SEditorViewport::OnFocusViewportToSelection();
}

TSharedRef<class SEditorViewport> SUnLive2DAnimBaseEditorViewport::GetViewportWidget()
{
	return SharedThis(this);
}

TSharedPtr<FExtender> SUnLive2DAnimBaseEditorViewport::GetExtenders() const
{
	TSharedPtr<FExtender> Result(MakeShareable(new FExtender));
	return Result;
}

void SUnLive2DAnimBaseEditorViewport::OnFloatingButtonClicked()
{

}

