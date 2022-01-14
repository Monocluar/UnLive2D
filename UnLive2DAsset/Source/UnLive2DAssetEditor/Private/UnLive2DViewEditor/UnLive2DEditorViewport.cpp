#include "UnLive2DEditorViewport.h"
#include "UnLive2DViewViewportClient.h"
#include "UnLive2DEditorCommands.h"
#include "SUnLive2DEditorViewportToolbar.h"
#include "Framework/MultiBox/MultiBoxExtender.h"

void SUnLive2DEditorViewport::Construct(const FArguments& InArgs)
{
	UnLive2DBeingEdited = InArgs._UnLive2DBeingEdited;

	SEditorViewport::Construct(SEditorViewport::FArguments());
}

void SUnLive2DEditorViewport::BindCommands()
{
	SEditorViewport::BindCommands();

	//const FUnLive2DEditorCommands& Commands = FUnLive2DEditorCommands::Get();
}

TSharedRef<FEditorViewportClient> SUnLive2DEditorViewport::MakeEditorViewportClient()
{
	EditorViewportClient = MakeShareable(new FUnLive2DViewViewportClient(UnLive2DBeingEdited.Get()));

	return EditorViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SUnLive2DEditorViewport::MakeViewportToolbar()
{
	return SNew(SUnLive2DEditorViewportToolbar, SharedThis(this));
}

EVisibility SUnLive2DEditorViewport::GetTransformToolbarVisibility() const
{
	return EVisibility::Visible;
}

void SUnLive2DEditorViewport::OnFocusViewportToSelection()
{
	EditorViewportClient->RequestFocusOnSelection(/*bInstant=*/ false);
}

TSharedRef<class SEditorViewport> SUnLive2DEditorViewport::GetViewportWidget()
{
	return SharedThis(this);
}

TSharedPtr<FExtender> SUnLive2DEditorViewport::GetExtenders() const
{
	TSharedPtr<FExtender> Result(MakeShareable(new FExtender));
	return Result;
}

void SUnLive2DEditorViewport::OnFloatingButtonClicked()
{

}

