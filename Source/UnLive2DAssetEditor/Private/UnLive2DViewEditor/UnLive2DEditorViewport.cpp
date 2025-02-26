#include "UnLive2DEditorViewport.h"
#include "UnLive2DViewViewportClient.h"
#include "SUnLive2DEditorViewportToolbar.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "UnLive2DViewEditor.h"
#include "UnLive2DEditorCommands.h"

void SUnLive2DEditorViewport::Construct(const FArguments& InArgs)
{
	UnLive2DBeingEdited = InArgs._UnLive2DBeingEdited;

	SEditorViewport::Construct(SEditorViewport::FArguments());
}

void SUnLive2DEditorViewport::BindCommands()
{
	SEditorViewport::BindCommands();

	const FUnLive2DEditorCommands& UnLive2DEditorCommands = FUnLive2DEditorCommands::Get();

	TSharedRef<FUnLive2DViewViewportClient> EditorViewportClientRef = EditorViewportClient.ToSharedRef();

	CommandList->MapAction(
		UnLive2DEditorCommands.SetShowGrid,
		FExecuteAction::CreateSP(EditorViewportClientRef, &FEditorViewportClient::SetShowGrid),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(EditorViewportClientRef, &FEditorViewportClient::IsSetShowGridChecked));

}

TSharedRef<FEditorViewportClient> SUnLive2DEditorViewport::MakeEditorViewportClient()
{
	EditorViewportClient = MakeShared<FUnLive2DViewViewportClient>(UnLive2DBeingEdited.Get());

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
