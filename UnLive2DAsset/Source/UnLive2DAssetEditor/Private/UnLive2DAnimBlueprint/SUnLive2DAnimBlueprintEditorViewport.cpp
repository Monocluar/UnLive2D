#include "SUnLive2DAnimBlueprintEditorViewport.h"
#include "UnLive2DAnimBlueprintViewportClient.h"
#include "Animation/UnLive2DAnimBlueprint.h"

void SUnLive2DAnimBlueprintEditorViewport::Construct(const FArguments& InArgs)
{
	UnLive2DAnimBlueprintEdited = InArgs._UnLive2DAnimBlueprintEdited;

	SEditorViewport::Construct(SEditorViewport::FArguments());
}

TSharedRef<FEditorViewportClient> SUnLive2DAnimBlueprintEditorViewport::MakeEditorViewportClient()
{
	EditorViewportClient = MakeShareable(new FUnLive2DAnimBlueprintViewportClient(UnLive2DAnimBlueprintEdited.Get()));

	return EditorViewportClient.ToSharedRef();
}

EVisibility SUnLive2DAnimBlueprintEditorViewport::GetTransformToolbarVisibility() const
{
	return EVisibility::Visible;
}

void SUnLive2DAnimBlueprintEditorViewport::OnFocusViewportToSelection()
{
	EditorViewportClient->RequestFocusOnSelection(/*bInstant=*/ false);

	SEditorViewport::OnFocusViewportToSelection();
}

TSharedRef<class SEditorViewport> SUnLive2DAnimBlueprintEditorViewport::GetViewportWidget()
{
	return SharedThis(this);
}

TSharedPtr<FExtender> SUnLive2DAnimBlueprintEditorViewport::GetExtenders() const
{
	TSharedPtr<FExtender> Result(MakeShareable(new FExtender));
	return Result;
}

void SUnLive2DAnimBlueprintEditorViewport::OnFloatingButtonClicked()
{

}
