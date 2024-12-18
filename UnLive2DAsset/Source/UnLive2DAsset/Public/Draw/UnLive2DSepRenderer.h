
#pragma once

#include "CoreMinimal.h"
#include "Misc/EngineVersionComparison.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/TextureRenderTarget2D.h"
#include "UnLive2DRendererComponent.h"
#include "Templates/SharedPointer.h"

namespace Live2D { namespace Cubism { namespace Framework 
{
	class CubismModel;
}}}


namespace Csm = Live2D::Cubism::Framework;

class UUnLive2DViewRendererUI;
class FUnLive2DRawModel;

#if ENGINE_MAJOR_VERSION < 5
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

namespace EUnLive2DRenderStateOwnerType
{
	enum Type
	{
		Component,

		UMG,

		ThumbnailRenderer
	};
}

struct FUnLive2DRenderBuffers
{
public:
	TMap<int32, FIndexUnLiveBufferRHIRef> IndexBuffers; // 索引缓冲
	TMap<int32, FVertexUnLiveBufferRHIRef> VertexBuffers; // 顶点缓冲
	TMap<int32, int32> VertexCounts; // 顶点数

public:
	void Clear()
	{
		IndexBuffers.Empty();
		VertexBuffers.Empty();
		VertexCounts.Empty();
	}
};

class UNLIVE2DASSET_API FUnLive2DRenderState : public TSharedFromThis<FUnLive2DRenderState>
{
public:

	FUnLive2DRenderState() {}

	FUnLive2DRenderState(UObject* InOwnerObject, UWorld* InWorld);

	~FUnLive2DRenderState();

public:

	void InitRender(TWeakObjectPtr<class UUnLive2D> InNewUnLive2D, TSharedPtr<FUnLive2DRawModel> InUnLive2DRawModel);

	void InitRender(const UUnLive2D* InNewUnLive2D, TSharedPtr<FUnLive2DRawModel> InUnLive2DRawModel);

	void NoLowPreciseMask(bool InVal) { bNoLowPreciseMask = InVal; }
	bool GetUseHighPreciseMask() const;

	// 图片加载
	void LoadTextures(TWeakPtr<FUnLive2DRawModel> InUnLive2DRawModel);

	// 释放图片
	void UnLoadTextures();

	// 释放老材质
	void UnOldMaterial();

	// 获取图片
	class UTexture2D* GetRandererStatesTexturesTextureIndex(const Csm::CubismModel* Live2DModel, const int32& DrawableIndex) const;

	// 获取当前的动态材质
	UMaterialInstanceDynamic* GetMaterialInstanceDynamicToIndex(const Csm::CubismModel* Live2DModel, const int32 DrawableIndex, bool bIsMesk);

	// 根据绘制索引ID获取当前的遮罩
	class CubismClippingContext* GetClipContextInDrawableIndex(const uint32 DrawableIndex) const;

	// 更新遮罩
	void UpdateRenderBuffers(TWeakPtr<FUnLive2DRawModel> InUnLive2DRawModel);

	// 更新遮罩纹理
	void UpdateMaskBufferRenderTarget(FRHICommandListImmediate& RHICmdList, Csm::CubismModel* tp_Model, ERHIFeatureLevel::Type FeatureLevel);

	// 
	void MaskFillVertexBuffer(Csm::CubismModel* tp_Model, const int32 drawableIndex, FVertexUnLiveBufferRHIRef ScratchVertexBufferRHI, FRHICommandListImmediate& RHICmdList);

	// 获取Live2D模型在遮罩矩阵中的矩阵
	FUnLiveMatrix GetUnLive2DPosToClipMartix(class CubismClippingContext* ClipContext, FUnLiveVector4& ChanelFlag);

	// 更新背景颜色
	void SetDynamicMaterialTintColor(FLinearColor& NewColor);

	// 设置渲染材质
	void SetUnLive2DMaterial(int32 InModeIndex, UMaterialInterface* NewUnLive2DMaterial);

	// 设置UnLive2D资源
	void SetUnLive2D(class UUnLive2D* InUnLive2D);

protected:

	// 初始化渲染
	void InitRenderBuffers(TSharedPtr<FUnLive2DRawModel> InUnLive2DRawModel);

	const UUnLive2D* GetUnLive2D() const;

	UMaterialInstanceDynamic* GetUnLive2DMaterial(int32 InModeIndex) const;

	FORCEINLINE FName GetDMaterialTextureParameterName() const;

	void UnLive2DFillMaskParameter(class CubismClippingContext* clipContext, FUnLiveMatrix& ts_MartixForMask, FUnLiveVector4& ts_BaseColor, FUnLiveVector4& ts_ChanelFlag);

private:

	// 渲染图片组
	TArray<TWeakObjectPtr<UTexture2D>> RandererStatesTextures;

	// 裁剪管理器
	TSharedPtr<class CubismClippingManager_UE> UnLive2DClippingManager;

	TWeakObjectPtr<UTextureRenderTarget2D> MaskBufferRenderTarget; //遮罩渲染缓冲图片

private:
	bool bNoLowPreciseMask; // 是否有高精度遮罩

	TMap<int32, UMaterialInstanceDynamic*> UnLive2DToNormalBlendMaterial; // CubismBlendMode_Normal普通动态材质
	TMap<int32, UMaterialInstanceDynamic*> UnLive2DToAdditiveBlendMaterial; // CubismBlendMode_Additive叠加动态材质
	TMap<int32, UMaterialInstanceDynamic*> UnLive2DToMultiplyBlendMaterial; // CubismBlendMode_Multiplicative乘积动态材质

	mutable FUnLive2DRenderBuffers MaskRenderBuffers;

	TWeakObjectPtr<const class UUnLive2D> SourceUnLive2D;

	bool bInitRenderBuffers;

	UWorld* World;

	UObject* OwnerObject;


	// Live2D颜色混合模式为CubismBlendMode_Normal使用的材质
	UMaterialInterface* UnLive2DNormalMaterial;

	// Live2D颜色混合模式为CubismBlendMode_Additive使用的材质
	UMaterialInterface* UnLive2DAdditiveMaterial;

	// Live2D颜色混合模式为CubismBlendMode_Multiplicative使用的材质
	UMaterialInterface* UnLive2DMultiplyMaterial;

};