
#pragma once

#include "CoreMinimal.h"
#include "UnLive2DViewEditor.h"
#include "Widgets/SCompoundWidget.h"

class SUnLive2DParameterGroup : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DParameterGroup) {}
	SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs, TSharedPtr<FUnLive2DViewEditor> InUnLive2DEditor);


private:

	TWeakPtr<class FUnLive2DViewEditor> UnLive2DEditorPtr;

};