
#pragma once
#include "UnLive2DCubismCore.h"
#include "PrimitiveSceneProxy.h"
#include "UnLive2DProxyBase.h"
#include "RenderResource.h"

class UUnLive2DRendererComponent;
class UTextureRenderTarget2D;
class CubismClippingManager_UE;
class FUnLive2DRawModel;
class UMaterialInstanceDynamic;
class FRHICommandListImmediate;
class UBodySetup;

namespace Live2D { namespace Cubism { namespace Framework 
{
	class CubismModel;
}}}

class FUnLive2DTargetBoxProxy final : public UnLive2DProxyBase ,public IUnLive2DRTRender
{

private:
	void UpdateSection_RenderThread(FRHICommandListImmediate& RHICmdList);

protected:
	virtual void OnUpData() override;
	virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials = false) const override;

	SIZE_T GetTypeHash() const override;
	virtual uint32 GetMemoryFootprint() const override;
	virtual bool CanBeOccluded() const override;

	// FPrimitiveSceneProxy interface.
	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
	//virtual void GetLightRelevance(const FLightSceneProxy* LightSceneProxy, bool& bDynamic, bool& bRelevant, bool& bLightMapped, bool& bShadowMapped) const override;
	// End of FPrimitiveSceneProxy interface.
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	void RenderCollision(UBodySetup* InBodySetup, FMeshElementCollector& Collector, int32 ViewIndex, const FEngineShowFlags& EngineShowFlags, const FBoxSphereBounds& InBounds, bool bRenderInEditor) const;
#endif

#if WITH_EDITOR
	virtual void UpDataUnLive2DProperty(FName PropertyName) override;
#endif

private:
	virtual const UTexture2D* GetTexture(const uint8& TextureIndex) const override;

public:

	FUnLive2DTargetBoxProxy(UUnLive2DRendererComponent* InComponent);
	~FUnLive2DTargetBoxProxy();

private:
	UTextureRenderTarget2D* RenderTarget; //渲染缓冲图片
	
	UMaterialInstanceDynamic* MaterialInstance; // 使用材质
	// 是否合批
	bool bCombinedbBatch;

	// 遮罩图
	FTextureRHIRef MaskBuffer;

	UBodySetup* BodySetup;
	FMaterialRelevance MaterialRelevance;
	FIntPoint CurrentDrawSize;
};

