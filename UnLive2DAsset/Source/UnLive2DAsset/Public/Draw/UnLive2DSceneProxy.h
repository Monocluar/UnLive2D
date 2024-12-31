// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "PrimitiveSceneProxy.h"
#include "DynamicMeshBuilder.h"

class UUnLive2DRendererComponent;
class FUnLive2DRawModel;
class CubismClippingManager_UE;

namespace Live2D { namespace Cubism { namespace Framework 
{
	class CubismModel;
}}}


namespace Csm = Live2D::Cubism::Framework;

class UNLIVE2DASSET_API FUnLive2DSceneProxy : public FPrimitiveSceneProxy
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
		TArray<int32> DrawableCounts;
		CubismClippingManager_UE* ClippingManager;

		uint8 ScaleMesh;


		//FUnLive2DDynamicMeshSectionData MeshSectionData;
	public:
		/** Material applied to this section */
		UMaterialInterface* Material;
		/** Vertex buffer for this section */
		FStaticMeshVertexBuffers VertexBuffers;
		FDynamicMeshIndexBuffer16 IndexBuffer;
		/** Vertex factory for this section */
		FLocalVertexFactory VertexFactory;


	public:
		FUnLive2DVertexBuffer(Csm::CubismModel* InUnLive2DRawModel, CubismClippingManager_UE* InClippingManager, const TArray<int32>& InDrawableCounts, ERHIFeatureLevel::Type InFeatureLevel, UMaterialInterface* InMaterial = nullptr);

	public:

		/* Moves all the PaperVertex data onto the RHI buffers. */
		void CommitVertexData(FRHICommandListBase& RHICmdList);
	public:

		void InitRHI(FRHICommandListBase& RHICmdList);
		void ReleaseResource();
		void UpdateSection_RenderThread(FRHICommandListBase& RHICmdList);
	protected:

		// 根据绘制索引ID获取当前的遮罩
		class CubismClippingContext* GetClipContextInDrawableIndex(const uint32 DrawableIndex) const;

		void GetUnLive2DVertexData(TArray<FDynamicMeshVertex>& OutVertices, TArray<uint16>& OutIndices) const;
	};

public:

	FUnLive2DSceneProxy(const UUnLive2DRendererComponent* InComponent);
	~FUnLive2DSceneProxy();

public:
	void OnUpData();

protected:
	SIZE_T GetTypeHash() const override;
	// FPrimitiveSceneProxy interface.
	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
	virtual uint32 GetMemoryFootprint() const override;
	//virtual void CreateRenderThreadResources(FRHICommandListBase& RHICmdList) override;

protected:

	bool GetMeshElement(FMeshElementCollector& Collector, int32 SectionIndex, uint8 DepthPriorityGroup, FUnLive2DVertexBuffer* InSectionData, FMeshBatch& OutMeshBatch) const;

	bool UpdataSections();

	void ClearSections();

	UMaterialInstance* GetMaterialInstanceDynamicToIndex(const int32& DrawableIndex);

protected:

	// Live2D模型设置模块
	TWeakPtr<FUnLive2DRawModel> UnLive2DRawModel;
	// 裁剪管理器
	TUniquePtr<CubismClippingManager_UE> UnLive2DClippingManager;

	TArray<FUnLive2DVertexBuffer*> Sections;

	// 是否合批
	bool bCombinedbBatch;
private:

	// 绘制Buffer数
	TArray<uint16> DrawableIndexList;
#if ENGINE_MAJOR_VERSION < 5
	const UUnLive2DRendererComponent* OwnerComponent;
#else
	TObjectPtr<const UUnLive2DRendererComponent> OwnerComponent;
#endif

#if ENGINE_MAJOR_VERSION < 5
	TMap<uint16, class UMaterialInstanceDynamic*> UnLive2DToBlendMaterialList;
#else
	TMap<uint16, TObjectPtr<class UMaterialInstanceDynamic>> UnLive2DToBlendMaterialList;
#endif

	friend class UUnLive2DRendererComponent;
};