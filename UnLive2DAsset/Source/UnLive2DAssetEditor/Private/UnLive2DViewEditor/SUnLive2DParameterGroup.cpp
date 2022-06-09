#include "SUnLive2DParameterGroup.h"
#include "UnLive2D.h"

void SUnLive2DParameterGroup::Construct(const FArguments& InArgs, TSharedPtr<FUnLive2DViewEditor> InUnLive2DEditor)
{
	UnLive2DEditorPtr = InUnLive2DEditor;


	if (!UnLive2DEditorPtr.IsValid()) return;

	UUnLive2D* UnLive2D = UnLive2DEditorPtr.Pin()->GetUnLive2DBeingEdited();

	if (UnLive2D == nullptr) return;

	UnLive2D->GetModelParamterGroup();

}

