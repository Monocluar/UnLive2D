// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "PrimitiveSceneProxy.h"
#include "DynamicMeshBuilder.h"
#include "UnLive2DProxyBase.h"
#include "StaticMeshResources.h"

class UUnLive2DRendererComponent;
class FUnLive2DRawModel;
class FUnLive2DRenderState;
class CubismClippingManager_UE;

namespace Live2D { namespace Cubism { namespace Framework 
{
	class CubismModel;
}}}


namespace Csm = Live2D::Cubism::Framework;

class UNLIVE2DASSET_API FUnLive2DSceneProxy : public UnLive2DProxyBase ,public IUnLive2DRenderBase
{
protected:

	struct FUnLive2DSectionData
	{
		TArray<int32> DrawableCounts;

		UMaterialInstance* Material;
	};

	// 顶点Buffer
	class FUnLive2DVertexBuffer
	{
	protected:
		Csm::CubismModel* UnLive2DRawModel;
		CubismClippingManager_UE* RenderState;

		uint8 ScaleMesh;

		FBox LocalBox;
		//FUnLive2DDynamicMeshSectionData MeshSectionData;

		mutable bool bMarkDirty;
	public:
		TArray<int32> DrawableCounts;
		/** Material applied to this section */
		UMaterialInterface* Material;
		/** Vertex buffer for this section */
		FStaticMeshVertexBuffers VertexBuffers;
		FDynamicMeshIndexBuffer16 IndexBuffer;
		/** Vertex factory for this section */
		FLocalVertexFactory VertexFactory;

#if WITH_EDITOR
		// 深度
		uint16 Depth;
#endif
	public:
		FUnLive2DVertexBuffer(Csm::CubismModel* InUnLive2DRawModel, CubismClippingManager_UE* InClippingManager, const TArray<int32>& InDrawableCounts, ERHIFeatureLevel::Type InFeatureLevel, uint8 InScaleMesh = 100,UMaterialInterface* InMaterial = nullptr);

	public:

		void InitRHI(FRHICommandListBase& RHICmdList);
		void ReleaseResource();
		void UpdateSection_RenderThread(FRHICommandListBase& RHICmdList);

		const FBox& GetLocalBox() const;

		void UpdateVertices();
	protected:

		void GetUnLive2DVertexData(TArray<FDynamicMeshVertex>& OutVertices, TArray<uint16>& OutIndices, FBox& OutLocalBox) const;

	};

public:

	FUnLive2DSceneProxy(UUnLive2DRendererComponent* InComponent);
	~FUnLive2DSceneProxy();

protected:
	virtual bool OnUpData() override;

	virtual void OnUpDataRenderer() override;

	SIZE_T GetTypeHash() const override;
	virtual bool CanBeOccluded() const override;
	// FPrimitiveSceneProxy interface.
	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
	virtual uint32 GetMemoryFootprint() const override;
	//virtual void CreateRenderThreadResources(FRHICommandListBase& RHICmdList) override;
	virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials = false) const override;
	virtual void OnComponetDestroy() override;

#if WITH_EDITOR
	virtual void UpDataUnLive2DProperty(FName PropertyName) override;
#endif
protected:

	bool GetMeshElement(FMeshElementCollector& Collector, int32 SectionIndex, uint8 DepthPriorityGroup, FUnLive2DVertexBuffer* InSectionData, FMeshBatch& OutMeshBatch) const;

	bool UpdataSections();

	void ClearSections();

	UMaterialInstance* GetMaterialInstanceDynamicToIndex(const int32& DrawableIndex);


protected:

	TArray<FUnLive2DVertexBuffer*> Sections;

	// 是否合批
	bool bCombinedbBatch;

	//class UBodySetup* BodySetup;

	FMaterialRelevance MaterialRelevance;
private:

#if ENGINE_MAJOR_VERSION < 5
	TMap<uint16, class UMaterialInstanceDynamic*> UnLive2DToBlendMaterialList;
#else
	TMap<uint16, TObjectPtr<class UMaterialInstanceDynamic>> UnLive2DToBlendMaterialList;
#endif

};