
#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "UnLive2DViewRendererUI.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Draw/IUnLive2DRenderBase.h"

class UUnLive2D;
class FUnLive2DRawModel;
class CubismClippingManager_UE;

DECLARE_DELEGATE_OneParam(FOnUpDataRender, TWeakPtr<FUnLive2DRawModel>);
DECLARE_DELEGATE_RetVal(TSharedRef<class FUnLive2DRenderState>, FOnInitUnLive2DRender);

class UNLIVE2DASSET_API SUnLive2DViewUI : public SLeafWidget, public IUnLive2DRTRender
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DViewUI)
		: _PlayRate(1.f)
		, _RenderTargetSize(1024)
		, _UnLive2DRenderType(EUnLive2DRenderType::RenderTarget)
		, _NormalMaterial()
		, _AdditiveMaterial()
		, _MultiplyMaterial()
		, _RTMaterial()
		{}
		SLATE_ATTRIBUTE(float, PlayRate)
		SLATE_ARGUMENT(int32, RenderTargetSize)
		SLATE_ARGUMENT(EUnLive2DRenderType, UnLive2DRenderType)
		SLATE_ARGUMENT(FSoftObjectPath, NormalMaterial)
		SLATE_ARGUMENT(FSoftObjectPath, AdditiveMaterial)
		SLATE_ARGUMENT(FSoftObjectPath, MultiplyMaterial)
		SLATE_ARGUMENT(FSoftObjectPath, RTMaterial)
	SLATE_END_ARGS()

public:
	void Construct( const FArguments& InArgs, const UUnLive2D* InUnLive2D);
	virtual ~SUnLive2DViewUI();
public:

	void SetUnLive2D(const UUnLive2D* InUnLive2D);

	void SetPlayRate(const TAttribute<float>& InValueAttribute);

	void SetLive2DRenderType(EUnLive2DRenderType InUnLive2DRenderType);

protected:

	void InitUnLive2D();

	void InitLive2DRenderType();

	void ClearRTCache();

	bool UpdataSections();

	// 描画
	void Flush(int32 LayerId, FSlateWindowElementList& OutDrawElements, bool bParentEnabled) const;

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	virtual const UTexture2D* GetTexture(const uint8& TextureIndex) const override;

	TSharedPtr<FSlateBrush> MakeSlateBrushByFlags(const uint8& Flags, const int32& InTextureIndex);

protected:
	// SWidget overrides
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	virtual FVector2D ComputeDesiredSize(float) const override;

private:
	void OnMotionPlayeEnd();

	FTextureRHIRef GetMaskTextureRHIRef() const;

public:

	TWeakObjectPtr<const UUnLive2D> SourceUnLive2DPtr;

private:

	struct FCustomVertsData
	{
	public:
		TArray<int32> DrawableCounts;

		TSharedPtr<FSlateBrush> Brush;

		uint8 Flags;
		uint8 TextureIndex;

		TArray<FSlateVertex> InterlottingLive2DVertexData;
		TArray<SlateIndex> InterlottingLive2DIndexData;

		uint16 CacheIndexCount;
		uint16 CacheVerticesIndexCount;
		uint8 MaskUID;
	public:
		FCustomVertsData()
			: Flags(0)
			, TextureIndex(0)
			, CacheIndexCount(0)
			, CacheVerticesIndexCount(0)
			, MaskUID(0)
		{}
	public:
		bool Equals(const uint8& InFlags, const uint8& InTextureIndex, const uint8& InMaskUID)
		{
			return (Flags == InFlags && TextureIndex == InTextureIndex && MaskUID == InMaskUID);
		}


		void UpDataVertsData(const FGeometry& AllottedGeometry,TSharedRef<FUnLive2DRawModel> UnLive2DRawModel, TWeakPtr<CubismClippingManager_UE> UnLive2DClippingManager);

	};

	TArray<FCustomVertsData> UnLive2DCustomVertsData;

	struct FUnLive2DSlateMaterialBrush : public FSlateBrush
	{
		FUnLive2DSlateMaterialBrush(UMaterialInterface* InMaterial, const FVector2D& InImageSize);

		FUnLive2DSlateMaterialBrush(UTexture* InTexture);

	public:
		EUnLive2DRenderType UnLive2DRenderType;

	};
	// 创建类型
	EUnLive2DRenderType UnLive2DRenderType;

	TWeakObjectPtr<class UTextureRenderTarget2D> MaskBufferRenderTarget; //遮罩渲染缓冲图片

	FTextureRHIRef MaskBuffer; // 遮罩图

	// 是否合批
	bool bCombinedbBatch;

	TAttribute<float> PlayRate;

	int32 RenderTargetSize;

private:
	FSoftObjectPath NormalMaterial;
	FSoftObjectPath AdditiveMaterial;
	FSoftObjectPath MultiplyMaterial;
	FSoftObjectPath RTMaterial;

	TMap<uint16, TSharedRef<FUnLive2DSlateMaterialBrush>> UnLive2DToBlendMaterialList;
	UTextureRenderTarget2D* RenderTarget; //渲染缓冲图片
};