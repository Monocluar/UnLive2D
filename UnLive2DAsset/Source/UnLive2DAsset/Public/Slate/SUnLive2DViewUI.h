
#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "UnLive2DViewRendererUI.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/GCObject.h"
#include "FWPort/UnLive2DRawModel.h"

class UUnLive2D;

DECLARE_DELEGATE_OneParam(FOnUpDataRender, TWeakPtr<FUnLive2DRawModel>);

class UNLIVE2DASSET_API SUnLive2DViewUI : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DViewUI)
	{}
	SLATE_EVENT(FOnUpDataRender, OnUpDataRender)
	SLATE_END_ARGS()

public:
	void Construct( const FArguments& InArgs , class UUnLive2DViewRendererUI* InRendererUI);

public:

	void SetUnLive2D(const UUnLive2D* InUnLive2D);

	void ReleaseRenderStateData();

	void PlayMotion(class UUnLive2DMotion* InMotion);

	void PlayExpression(class UUnLive2DExpression* InExpression);

	void StopMotion();

protected:

	void InitUnLive2D();

	void UpDateMesh(const FGeometry &AllottedGeometry, int32 DrawableIndex, class CubismClippingContext* ClipContext, const FWidgetStyle& InWidgetStyle);

	// 描画
	void Flush(int32 LayerId, FSlateWindowElementList& OutDrawElements, bool bParentEnabled);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

protected:
	// SWidget overrides
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	virtual FVector2D ComputeDesiredSize(float) const override;
public:

	TWeakObjectPtr<class UUnLive2DViewRendererUI> OwnerWidget;

	// Live2D模型设置模块
	TSharedPtr<class FUnLive2DRawModel> UnLive2DRawModel;

	// 渲染回调
	FOnUpDataRender OnUpDataRender;
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


	friend UUnLive2DViewRendererUI;

};