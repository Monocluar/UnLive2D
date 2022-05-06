#include "Slate/SUnLive2DViewUI.h"

void SUnLive2DViewUI::Construct(const FArguments& InArgs)
{
	UnLive2DAsset = InArgs._UnLive2D;
}

void SUnLive2DViewUI::SetUnLive2D(TAttribute<const UUnLive2D*> InUnLive2D)
{
	UnLive2DAsset = InUnLive2D;
}

int32 SUnLive2DViewUI::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	return LayerId;
}

