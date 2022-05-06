
#pragma once

#include "CoreMinimal.h"
#include "Model/CubismModel.hpp"
#include "Type/CubismBasicType.hpp"
#include "CubismFramework.hpp"
#include "Misc/EngineVersionComparison.h"

class UUnLive2DRendererComponent;

#if UE_VERSION_OLDER_THAN(5,0,0)
typedef FMatrix FUnLiveMatrix;
typedef FVector4 FUnLiveVector4;
typedef FIndexBufferRHIRef FIndexUnLiveBufferRHIRef;
typedef FVertexBufferRHIRef FVertexUnLiveBufferRHIRef;
#else
typedef FMatrix44f FUnLiveMatrix;
typedef FVector4f FUnLiveVector4;
typedef FBufferRHIRef FIndexUnLiveBufferRHIRef;
typedef FBufferRHIRef FVertexUnLiveBufferRHIRef;
#endif


struct FUnLive2DRenderBuffers
{
public:
	TMap<int32, FIndexUnLiveBufferRHIRef> IndexBuffers; // 索引缓冲
	TMap<int32, FVertexUnLiveBufferRHIRef> VertexBuffers; // 顶点缓冲
	TMap<int32, int32> VertexCounts; // 顶点数
};

struct FUnLive2DRenderState
{
public:

	FUnLive2DRenderState() {}

	FUnLive2DRenderState(UUnLive2DRendererComponent* InComp);

	~FUnLive2DRenderState();

public:
	void InitRender(TWeakObjectPtr<class UUnLive2D> InNewUnLive2D);

	void NoLowPreciseMask(bool InVal) { bNoLowPreciseMask = InVal; }
	bool GetUseHighPreciseMask() const;

	// 图片加载
	void LoadTextures();

	// 释放图片
	void UnLoadTextures();

	// 获取图片
	class UTexture2D* GetRandererStatesTexturesTextureIndex(Csm::CubismModel* Live2DModel, const Csm::csmInt32& DrawableIndex) const;

	// 获取当前的动态材质
	UMaterialInstanceDynamic* GetMaterialInstanceDynamicToIndex(Csm::CubismModel* Live2DModel, const Csm::csmInt32 DrawableIndex, bool bIsMesk);

	// 根据绘制索引ID获取当前的遮罩
	class CubismClippingContext* GetClipContextInDrawableIndex(const Csm::csmUint32 DrawableIndex) const;

	// 更新遮罩
	void UpdateRenderBuffers();

	// 更新遮罩纹理
	void UpdateMaskBufferRenderTarget(FRHICommandListImmediate& RHICmdList, Csm::CubismModel* tp_Model, ERHIFeatureLevel::Type FeatureLevel);

	// 
	void MaskFillVertexBuffer(Csm::CubismModel* tp_Model, const Csm::csmInt32 drawableIndex, FVertexUnLiveBufferRHIRef ScratchVertexBufferRHI, FRHICommandListImmediate& RHICmdList);

	// 获取Live2D模型在遮罩矩阵中的矩阵
	FUnLiveMatrix GetUnLive2DPosToClipMartix(class CubismClippingContext* ClipContext, FUnLiveVector4& ChanelFlag);

	// 更新背景颜色
	void SetDynamicMaterialTintColor(FLinearColor& NewColor);

protected:

	// 初始化渲染
	void InitRenderBuffers();

private:

	// 渲染图片组
	TArray<TWeakObjectPtr<UTexture2D>> RandererStatesTextures;

	// 裁剪管理器
	TSharedPtr<class CubismClippingManager_UE> UnLive2DClippingManager;

	TWeakObjectPtr<UTextureRenderTarget2D> MaskBufferRenderTarget; //遮罩渲染缓冲图片

private:
	bool bNoLowPreciseMask; // 是否有高精度遮罩

	TWeakObjectPtr<UUnLive2DRendererComponent> OwnerCompWeak;

	TMap<int32, UMaterialInstanceDynamic*> UnLive2DToNormalBlendMaterial; // CubismBlendMode_Normal普通动态材质
	TMap<int32, UMaterialInstanceDynamic*> UnLive2DToAdditiveBlendMaterial; // CubismBlendMode_Additive叠加动态材质
	TMap<int32, UMaterialInstanceDynamic*> UnLive2DToMultiplyBlendMaterial; // CubismBlendMode_Multiplicative乘积动态材质

	TSharedPtr<FUnLive2DRenderBuffers> MaskRenderBuffers;

};