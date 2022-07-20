
#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "UnLive2DViewRendererUI.h"
#include "Materials/MaterialInstanceDynamic.h"

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

	virtual ~SUnLive2DViewUI();

public:

	void SetUnLive2D(const UUnLive2D* InUnLive2D);

	const UUnLive2D* GetUnLive2D() const;

	void ReleaseRenderStateData();

protected:

	void InitUnLive2D();

	void UpDateMesh(const FGeometry &AllottedGeometry, int32 DrawableIndex, class CubismClippingContext* ClipContext);

	// 描画
	void Flush(int32 LayerId, FSlateWindowElementList& OutDrawElements);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

protected:
	// SWidget overrides
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	virtual FVector2D ComputeDesiredSize(float) const override;
public:

	TWeakObjectPtr<class UUnLive2DViewRendererUI> OwnerWidget;

protected:
	// Live2D资源
	TWeakObjectPtr<const UUnLive2D> UnLive2DWeak;

	// UnLive2D 渲染模块
	TSharedPtr<class FUnLive2DRenderState> UnLive2DRenderPtr;

	// Live2D模型设置模块
	TSharedPtr<class FUnLive2DRawModel> UnLive2DRawModel;
private:

	struct FCustomVertsData
	{
	public:
		TArray<FSlateVertex> InterlottingLive2DVertexData;

		TArray<SlateIndex> InterlottingLive2DIndexData;

		UMaterialInstanceDynamic* InterlottingDynamicMat;

	public:
		
		FCustomVertsData(TArray<FSlateVertex>& Live2DVertexData, TArray<SlateIndex> Live2DIndexData, UMaterialInstanceDynamic* DynamicMat)
			: InterlottingLive2DVertexData(MoveTemp(Live2DVertexData))
			, InterlottingLive2DIndexData(MoveTemp(Live2DIndexData))
			, InterlottingDynamicMat(DynamicMat)
		{
		}

		FCustomVertsData()
			: InterlottingDynamicMat(nullptr)
		{}
	};

	TArray<FCustomVertsData> UnLive2DCustomVertsData;

};