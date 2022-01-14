#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "PrimitiveSceneProxy.h"

class FUnLive2DSceneProxy final : public FPrimitiveSceneProxy
{
public:

	SIZE_T GetTypeHash() const override;

	FUnLive2DSceneProxy(class UUnLive2DComponent* InComp);

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, class FMeshElementCollector& Collector) const override;

protected:

	void RenderCollision(UBodySetup* InBodySetup, FMeshElementCollector& Collector, int32 ViewIndex, const FEngineShowFlags& EngineShowFlags, const FBoxSphereBounds& InBounds, bool bRenderInEditor) const;

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;

	//virtual void GetLightRelevance(const FLightSceneProxy* LightSceneProxy, bool& bDynamic, bool& bRelevant, bool& bLightMapped, bool& bShadowMapped) const override;

	virtual void OnTransformChanged() override;

	virtual bool CanBeOccluded() const override;

	virtual uint32 GetMemoryFootprint(void) const override;

	FORCEINLINE uint32 GetAllocatedSize(void) const;

private:
	FVector2D Pivot;
	class UTextureRenderTarget2D* RenderTarget;
	class UMaterialInstanceDynamic* MaterialInstance;
	FMaterialRelevance MaterialRelevance;
	UBodySetup* BodySetup;
	FVector Origin;
};