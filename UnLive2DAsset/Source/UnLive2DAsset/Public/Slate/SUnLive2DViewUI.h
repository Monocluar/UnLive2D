
#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "Widgets/SLeafWidget.h"

class UUnLive2D;

class UNLIVE2DASSET_API SUnLive2DViewUI : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DViewUI)
		: _UnLive2D(nullptr)
	{}

		SLATE_ATTRIBUTE( const UUnLive2D*, UnLive2D)

	SLATE_END_ARGS()

public:
	SUnLive2DViewUI()
	{
		SetCanTick(false);
		bCanSupportFocus = false;
	}

	void Construct( const FArguments& InArgs );

public:

	void SetUnLive2D(TAttribute<const UUnLive2D*> InUnLive2D);

protected:
	// SWidget overrides
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

protected:
	// Live2D资源
	TAttribute<const UUnLive2D*> UnLive2DAsset;

};