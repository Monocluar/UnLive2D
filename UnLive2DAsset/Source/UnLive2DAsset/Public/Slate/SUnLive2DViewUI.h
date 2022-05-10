
#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "Widgets/SLeafWidget.h"

class UUnLive2D;

class UNLIVE2DASSET_API SUnLive2DViewUI : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DViewUI)
		: _OwnerWidget(nullptr)
	{}
	SLATE_ARGUMENT(UUnLive2DViewRendererUI*, OwnerWidget)
	SLATE_END_ARGS()

public:
	void Construct( const FArguments& InArgs );

public:

	void SetUnLive2D(const UUnLive2D* InUnLive2D);

	const UUnLive2D* GetUnLive2D() const;

protected:

	void InitUnLive2D();

	void UpDateMesh(int32 LayerId, FSlateWindowElementList &OutDrawElements, const FGeometry &AllottedGeometry, int32 DrawableIndex, class CubismClippingContext* ClipContext, int32& ElementIndex);

	virtual void Tick(float DeltaTime) const;

protected:
	// SWidget overrides
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

public:

	TWeakObjectPtr<class UUnLive2DViewRendererUI> OwnerWidget;

protected:
	// Live2D资源
	TWeakObjectPtr<const UUnLive2D> UnLive2DWeak;

	// UnLive2D 渲染模块
	TSharedPtr<struct FUnLive2DRenderState> UnLive2DRenderPtr;

private:

	/** 重置时间记录 */
	void ConstructSequence();

private:
	/** The sequence to drive the spinning animation */
	FCurveSequence Sequence;
	FCurveHandle Curve;

	float LastTime;
};