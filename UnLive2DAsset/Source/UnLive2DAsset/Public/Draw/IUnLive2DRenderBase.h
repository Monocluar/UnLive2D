#pragma once
#include "UnLive2DCubismCore.h"

enum EUnLive2DShaderFlags : uint8
{
	BlendMode_Nor = 1 << 0,
	BlendMode_Add = 1 << 1,
	BlendMode_Mul = 1 << 2,
	Mesk = 1 << 3,
	InvertedMesk = 1 << 4,
};

class IUnLive2DRenderBase
{

protected:

	// 查看绘制面层级是否改变
	bool UpDataDrawableIndexList(TArray<uint16>& OutSortedDrawableIndexList);

	// 创建裁切管理器
	bool CreateClippingManager();

	// 查看该绘制索引面的Flag
	uint8 GetUnLive2DShaderFlagsByDrawableIndex(const uint16& InDrawableIndex, uint8& MaskID) const;

	// <Drawable如果不是显示状态
	bool GetDrawableDynamicIsVisible(const uint16& InDrawableIndex) const;

	// 查看缓存索引面是否有变换
	bool IsCombinedbBatchDidChange(const TArray<int32>& InDrawableCounts) const;

	void ClearRenderBaseData();

private:
	// 绘制Buffer数
	TArray<uint16> DrawableIndexList;

protected:
	// Live2D模型设置模块
	TSharedPtr<class FUnLive2DRawModel> UnLive2DRawModel;
	// 裁剪管理器
	TSharedPtr<class CubismClippingManager_UE> UnLive2DClippingManager;
};

class IUnLive2DRTRender : public IUnLive2DRenderBase
{
public:
	virtual ~IUnLive2DRTRender(){};

public:
	class FUnLive2DTargetVertexDeclaration : public FRenderResource
	{
	public:

		FVertexDeclarationRHIRef VertexDeclarationRHI;

		virtual ~FUnLive2DTargetVertexDeclaration() {}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 4
		virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
#else
		virtual void InitRHI() override;
#endif
		virtual void ReleaseRHI() override;
	};

protected:

	struct FUnLive2DRenderTargetVertex
	{
		FULVector2f Position;
		FULVector2f UV;
		FLinearColor Color;
	};

	struct FUnLive2DRTSectionData
	{
		TArray<int32> DrawableCounts;
		uint8 Flags;
		uint8 TextureIndex;
		FUIBufferRHIRef CacheIndexBuffersRHI;
		FUVBufferRHIRef CacheVertexBufferRHI;
		uint16 CacheIndexCount;
		uint16 CacheVerticesIndexCount;
		uint8 MaskUID;

		~FUnLive2DRTSectionData();
	public:
		bool Equals(const uint8& InFlags, const uint8& InTextureIndex, const uint8& InMaskUID)
		{
			return (Flags == InFlags && TextureIndex == InTextureIndex && MaskUID == InMaskUID);
		}
	};

protected:
	uint16 GetVertexBySectionData(FRHICommandListImmediate& RHICmdList, FUnLive2DRTSectionData& InSectionData);

	bool UpdataRTSections(FRHICommandListImmediate& RHICmdList, bool bCombinedbBatch = true);

	void DrawSeparateToRenderTarget_RenderThread(FRHICommandListImmediate& RHICmdList, FTextureRenderTargetResource* OutTextureRenderTargetResource, ERHIFeatureLevel::Type FeatureLevel, FTextureRHIRef InMaskBuffer);

	void ClearSectionData();

protected:
	virtual const UTexture2D* GetTexture(const uint8& TextureIndex) const = 0;

protected:
	// 当前绘制使用数据
	TArray<FUnLive2DRTSectionData> UnLive2DSectionDataArr;
};
