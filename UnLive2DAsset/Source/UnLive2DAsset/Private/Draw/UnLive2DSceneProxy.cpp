#include "Draw/UnLive2DSceneProxy.h"
#include "UnLive2DRendererComponent.h"
#include "Stats/Stats2.h"
#include "UnLive2DAssetModule.h"
#include "FWPort/UnLive2DRawModel.h"
#include "FWPort/UnLive2DModelRender.h"
#include "MaterialDomain.h"
#include "PrimitiveUniformShaderParametersBuilder.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Rendering/StaticLightingSystemInterface.h"
#include "Draw/UnLive2DSepRenderer.h"

DECLARE_CYCLE_STAT(TEXT("UnLive2DSceneProxy GDME"), STAT_UnLive2DSceneProxy_GetDynamicMeshElements, STATGROUP_UnLive2D);
DECLARE_CYCLE_STAT(TEXT("UnLive2DVertexBuffer GDME"), STAT_UnLive2DVertexBuffer_UpdateSection_RenderThread, STATGROUP_UnLive2D);

SIZE_T FUnLive2DSceneProxy::GetTypeHash() const
{
	static size_t UniquePointer;
	return reinterpret_cast<size_t>(&UniquePointer);
}

FUnLive2DSceneProxy::FUnLive2DSceneProxy(UUnLive2DRendererComponent* InComponent)
	: UnLive2DProxyBase(InComponent)
	, bCombinedbBatch(true)
{
	if (!UnLive2DRawModel.IsValid()) return;
	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel.Pin()->GetModel();
	if (UnLive2DModel == nullptr) return;

	TSharedRef<FUnLive2DRawModel> UnLive2DRawModelRef = UnLive2DRawModel.Pin().ToSharedRef();
	UnLive2DRenderState = MakeUnique<FUnLive2DRenderState>(InComponent->GetUnLive2D(), UnLive2DRawModelRef);

	//Rendering::CubismRenderer::CubismBlendMode
	UpdataSections();
}

FUnLive2DSceneProxy::~FUnLive2DSceneProxy()
{
	ClearSections();
	for (auto& Item : UnLive2DToBlendMaterialList)
	{
		if (Item.Value == nullptr) continue;

		Item.Value->RemoveFromRoot();
		Item.Value->OnRemovedAsOverride(OwnerComponent.Get());
	}
	UnLive2DToBlendMaterialList.Empty();
}

void FUnLive2DSceneProxy::OnUpData()
{
	if (!UpdataSections() && Sections.Num() > 0)
	{
		ENQUEUE_RENDER_COMMAND(UnLive2DSceneProxy_UpdataSections)(
			[this](FRHICommandListImmediate& RHICmdList)
			{
				ERHIFeatureLevel::Type FeatureLevel = GetScene().GetFeatureLevel();
				UnLive2DRenderState->InitRHI(RHICmdList, FeatureLevel);
				/*for (int32 i = 0; i < Sections.Num(); i++)
				{
					Sections[i]->UpdateSection_RenderThread(RHICmdList);
				}
				*/
			});
	}
}

void FUnLive2DSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
{
	SCOPE_CYCLE_COUNTER(STAT_UnLive2DSceneProxy_GetDynamicMeshElements);

	if (!UnLive2DRawModel.IsValid()) return;

	const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;
	//Go for each section, creating a batch and collecting it
	for (int32 SectionIndex = 0; SectionIndex < Sections.Num(); SectionIndex++)
	{
		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				const FSceneView* View = Views[ViewIndex];

				const uint8 DPG = GetDepthPriorityGroup(View);
				const bool bIsWireframeView = View->Family->EngineShowFlags.Wireframe;

				FMeshBatch& Batch = Collector.AllocateMesh();
				Batch.bWireframe = bWireframe;
				FMaterialRenderProxy* MaterialProxy = Sections[SectionIndex]->Material->GetRenderProxy();
				if (GetMeshElement(Collector, SectionIndex, DPG, Sections[SectionIndex], Batch))
				{
					Collector.AddMesh(ViewIndex, Batch);
				}
			}
		}
		
	}

}


FPrimitiveViewRelevance FUnLive2DSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	const FEngineShowFlags& EngineShowFlags = View->Family->EngineShowFlags;

	checkSlow(IsInParallelRenderingThread());

	FPrimitiveViewRelevance Result;
	Result.bDrawRelevance = IsShown(View);
	Result.bShadowRelevance = IsShadowCast(View);
	Result.bDynamicRelevance = true;
	Result.bRenderInMainPass = ShouldRenderInMainPass();
	Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
	Result.bRenderCustomDepth = ShouldRenderCustomDepth();
	Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
	//MaterialRelevance.SetPrimitiveViewRelevance(Result);
	Result.bVelocityRelevance = DrawsVelocity() && Result.bOpaque && Result.bRenderInMainPass;
	return Result;
}

uint32 FUnLive2DSceneProxy::GetMemoryFootprint() const
{
	return sizeof(*this) + FPrimitiveSceneProxy::GetAllocatedSize();
}

void FUnLive2DSceneProxy::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials /*= false*/) const
{
	for (auto& Item : UnLive2DToBlendMaterialList)
	{
		if (Item.Value == nullptr) continue;
		OutMaterials.Add(Item.Value);
	}
}

bool FUnLive2DSceneProxy::GetMeshElement(FMeshElementCollector& Collector, int32 SectionIndex, uint8 DepthPriorityGroup, FUnLive2DVertexBuffer* InSectionData, FMeshBatch& Mesh) const
{
	if (InSectionData == nullptr) return false;

	checkSlow(VertexBuffer.IsInitialized() && VertexFactory.IsInitialized());
	FMeshBatchElement& BatchElement = Mesh.Elements[0];
	BatchElement.IndexBuffer = &InSectionData->IndexBuffer;
	Mesh.VertexFactory = &InSectionData->VertexFactory;
	Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
	Mesh.Type = PT_TriangleList;
	Mesh.DepthPriorityGroup = SDPG_World;
	Mesh.bCanApplyViewModeOverrides = false;
	Mesh.MaterialRenderProxy = InSectionData->Material->GetRenderProxy();

	{
		FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
		FPrimitiveUniformShaderParametersBuilder Builder;
		BuildUniformShaderParameters(Builder);
		DynamicPrimitiveUniformBuffer.Set(Collector.GetRHICommandList(), Builder);

		BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;
	}

	BatchElement.FirstIndex = 0;
	BatchElement.NumPrimitives = InSectionData->IndexBuffer.Indices.Num() / 3;
	BatchElement.MinVertexIndex = 0;
	BatchElement.MaxVertexIndex = InSectionData->VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;

	return true;
}

void FUnLive2DSceneProxy::ClearSections()
{
	for (FUnLive2DVertexBuffer* VertexBuffer : Sections)
	{
		if (VertexBuffer == nullptr) continue;

		VertexBuffer->ReleaseResource();
		delete VertexBuffer;
	}
	Sections.Empty();
}

UMaterialInstance* FUnLive2DSceneProxy::GetMaterialInstanceDynamicToIndex(const int32& DrawableIndex)
{
	if (!UnLive2DRawModel.IsValid()) return nullptr;
	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel.Pin()->GetModel();
	if (UnLive2DModel == nullptr) return nullptr;
	Rendering::CubismRenderer::CubismBlendMode BlendMode = UnLive2DModel->GetDrawableBlendMode(DrawableIndex);

	const csmInt32 TextureIndex = UnLive2DModel->GetDrawableTextureIndices(DrawableIndex);
	uint16 BlendModeIndex = BlendMode * 100;
	uint16 MapIndex = BlendModeIndex + TextureIndex;
	UMaterialInstanceDynamic* Material = nullptr;
	TObjectPtr<UMaterialInstanceDynamic> const* FindMaterial = UnLive2DToBlendMaterialList.Find(MapIndex);
	if (FindMaterial) return *FindMaterial;
	FSoftObjectPath UnLive2DMaterialSoftObject;
	switch (BlendMode)
	{
	case Rendering::CubismRenderer::CubismBlendMode_Normal:
		UnLive2DMaterialSoftObject = OwnerComponent->UnLive2DNormalMaterial;
	break;
	case Rendering::CubismRenderer::CubismBlendMode_Additive:
		UnLive2DMaterialSoftObject = OwnerComponent->UnLive2DAdditiveMaterial;
	break;
	case Rendering::CubismRenderer::CubismBlendMode_Multiplicative:
		UnLive2DMaterialSoftObject = OwnerComponent->UnLive2DMultiplyMaterial;
	break;
	}
	if (UnLive2DMaterialSoftObject.IsNull()) return nullptr;
	
	if (UMaterialInterface* MaterialInterface = Cast<UMaterialInterface>(UnLive2DMaterialSoftObject.TryLoad()))
	{
		UUnLive2DRendererComponent* RendererComponent = const_cast<UUnLive2DRendererComponent*>(OwnerComponent.Get());
		Material = UMaterialInstanceDynamic::Create(MaterialInterface, RendererComponent);
		if (RendererComponent->GetUnLive2D() == nullptr || !RendererComponent->GetUnLive2D()->TextureAssets.IsValidIndex(TextureIndex))
			return nullptr;
		Material->SetTextureParameterValue(TEXT("UnLive2D"), RendererComponent->GetUnLive2D()->TextureAssets[TextureIndex]);
		Material->SetTextureParameterValue(TEXT("UnLive2DMask"), UnLive2DRenderState->MaskBufferRenderTarget.Get());

		Material->OnAssignedAsOverride(RendererComponent);
		Material->AddToCluster(RendererComponent, true);
#if WITH_EDITOR
		if (!RendererComponent->IsCompiling())
		{
			FStaticLightingSystemInterface::OnPrimitiveComponentUnregistered.Broadcast(RendererComponent);
			if (RendererComponent->HasValidSettingsForStaticLighting(false))
			{
				FStaticLightingSystemInterface::OnPrimitiveComponentRegistered.Broadcast(RendererComponent);
			}
		}
#endif
	}
	if (Material)
	{
		Material->AddToRoot();
		UnLive2DToBlendMaterialList.Add(MapIndex, Material);
	}


	return Material;
}

bool FUnLive2DSceneProxy::UpdataSections()
{
	TArray<uint16> SortedDrawableIndexList;
	if (!UpDataDrawableIndexList(SortedDrawableIndexList)) return false;

	ClearSections();

	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel.Pin()->GetModel();
	// 当前绘制使用数据
	TArray<FUnLive2DSectionData> UnLive2DSectionDataArr;

	ERHIFeatureLevel::Type FeatureLevel = GetScene().GetFeatureLevel();
	// 获取绘制数据
	{
		for (csmInt32 i = 0; i < SortedDrawableIndexList.Num(); i++)
		{
			const csmInt32 DrawableIndex = SortedDrawableIndexList[i];
			// <Drawable如果不是显示状态，则通过处理
			if (!UnLive2DModel->GetDrawableDynamicFlagIsVisible(DrawableIndex)) continue;

			if (0 == UnLive2DModel->GetDrawableVertexIndexCount(DrawableIndex)) continue;

			csmFloat32 Opacity = UnLive2DModel->GetDrawableOpacity(DrawableIndex); // 获取不透明度
			if (Opacity <= 0.f) continue;
			UMaterialInstance* BlendMat = GetMaterialInstanceDynamicToIndex(DrawableIndex);

			FUnLive2DSectionData* MeshSectionData = nullptr;
			if (!bCombinedbBatch || UnLive2DSectionDataArr.Num() == 0 || UnLive2DSectionDataArr.Top().Material != BlendMat)
			{
				MeshSectionData = &UnLive2DSectionDataArr.AddDefaulted_GetRef();
				MeshSectionData->Material = BlendMat;
			}
			else
				MeshSectionData = &UnLive2DSectionDataArr.Top();

			MeshSectionData->DrawableCounts.Add(DrawableIndex);
		}
	}
#if WITH_EDITOR
	if (UnLive2DSectionDataArr.Num() > 0)
	{
		TArray<UMaterialInterface*> InUsedMaterialsForVerification;
		for (auto& Item : UnLive2DToBlendMaterialList)
		{
			if (Item.Value == nullptr) continue;
			InUsedMaterialsForVerification.Add(Item.Value);
		}
		SetUsedMaterialForVerification(InUsedMaterialsForVerification);
	}
#endif

	// 获取渲染数据
	{
		// Copy each section
		const int32 NumSections = UnLive2DSectionDataArr.Num();
		Sections.AddZeroed(NumSections);
		for (int32 SectionIdx = 0; SectionIdx < NumSections; SectionIdx++)
		{
			FUnLive2DVertexBuffer* NewSection = new FUnLive2DVertexBuffer(UnLive2DModel, UnLive2DRenderState.Get(), UnLive2DSectionDataArr[SectionIdx].DrawableCounts, FeatureLevel, UnLive2DSectionDataArr[SectionIdx].Material);
			Sections[SectionIdx] = NewSection;
#if WITH_EDITOR
			//NewSection->Depth = SectionIdx * 5;
			NewSection->Depth = 0;
#endif
			
		}
		ENQUEUE_RENDER_COMMAND(UnLive2DSceneProxy_UpdataSections)(
			[this, FeatureLevel](FRHICommandListImmediate& RHICmdList)
			{
				UnLive2DRenderState->InitRHI(RHICmdList, FeatureLevel);
				for (int32 i = 0; i < Sections.Num(); i++)
				{
					Sections[i]->InitRHI(RHICmdList);
				}
			});
	}

	return true;
}

FUnLive2DSceneProxy::FUnLive2DVertexBuffer::FUnLive2DVertexBuffer(Csm::CubismModel* InUnLive2DRawModel, FUnLive2DRenderState* InClippingManager, const TArray<int32>& InDrawableCounts, ERHIFeatureLevel::Type InFeatureLevel, UMaterialInterface* InMaterial)
	: UnLive2DRawModel(InUnLive2DRawModel)
	, DrawableCounts(InDrawableCounts)
	, RenderState(InClippingManager)
	, ScaleMesh(100)
	, Material(InMaterial)
	, VertexFactory(InFeatureLevel, "FProcMeshProxySection")
{
	if (Material == nullptr)
	{
		Material = UMaterial::GetDefaultMaterial(MD_Surface);
	}
}

void FUnLive2DSceneProxy::FUnLive2DVertexBuffer::CommitVertexData(FRHICommandListBase& RHICmdList)
{
	ReleaseResource();
	TArray<FDynamicMeshVertex> Vertices;
	GetUnLive2DVertexData(Vertices, IndexBuffer.Indices);
	VertexBuffers.InitFromDynamicVertex(&VertexFactory, Vertices, 4);

	VertexBuffers.PositionVertexBuffer.InitResource(RHICmdList);
	VertexBuffers.StaticMeshVertexBuffer.InitResource(RHICmdList);
	VertexBuffers.ColorVertexBuffer.InitResource(RHICmdList);
	IndexBuffer.InitResource(RHICmdList);
	VertexFactory.InitResource(RHICmdList);
}

void FUnLive2DSceneProxy::FUnLive2DVertexBuffer::InitRHI(FRHICommandListBase& RHICmdList)
{
	CommitVertexData(RHICmdList);
}

void FUnLive2DSceneProxy::FUnLive2DVertexBuffer::ReleaseResource()
{
	VertexBuffers.PositionVertexBuffer.ReleaseResource();
	VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();;
	VertexBuffers.ColorVertexBuffer.ReleaseResource();;
	IndexBuffer.ReleaseResource();
	VertexFactory.ReleaseResource();
}

void FUnLive2DSceneProxy::FUnLive2DVertexBuffer::UpdateSection_RenderThread(FRHICommandListBase& RHICmdList)
{
	SCOPE_CYCLE_COUNTER(STAT_UnLive2DVertexBuffer_UpdateSection_RenderThread);

	TArray<uint16> Indices;
	TArray<FDynamicMeshVertex> Vertices;
	GetUnLive2DVertexData(Vertices, IndexBuffer.Indices);

	if (Vertices.Num() == 0) return;
	VertexBuffers.PositionVertexBuffer.Init(Vertices.Num());
	VertexBuffers.StaticMeshVertexBuffer.Init(Vertices.Num(), 4);
	VertexBuffers.ColorVertexBuffer.Init(Vertices.Num());

	for (int32 i = 0; i < Vertices.Num(); i++)
	{
		const FDynamicMeshVertex& Vertex = Vertices[i];

		VertexBuffers.PositionVertexBuffer.VertexPosition(i) = Vertex.Position;
		VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(i, Vertex.TangentX.ToFVector3f(), Vertex.GetTangentY(), Vertex.TangentZ.ToFVector3f());
		VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 0, Vertex.TextureCoordinate[0]);
		VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 1, Vertex.TextureCoordinate[1]);
		VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 2, Vertex.TextureCoordinate[2]);
		VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 3, Vertex.TextureCoordinate[3]);
		VertexBuffers.ColorVertexBuffer.VertexColor(i) = Vertex.Color;
	}

	{
		auto& VertexBuffer = VertexBuffers.PositionVertexBuffer;
		void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
		RHICmdList.UnlockBuffer(VertexBuffer.VertexBufferRHI);
	}
	{
		auto& VertexBuffer = VertexBuffers.ColorVertexBuffer;
		void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
		RHICmdList.UnlockBuffer(VertexBuffer.VertexBufferRHI);
	}

	{
		auto& VertexBuffer = VertexBuffers.StaticMeshVertexBuffer;
		void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetTangentSize(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTangentData(), VertexBuffer.GetTangentSize());
		RHICmdList.UnlockBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI);
	}

	{
		auto& VertexBuffer = VertexBuffers.StaticMeshVertexBuffer;
		void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetTexCoordSize(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTexCoordData(), VertexBuffer.GetTexCoordSize());
		RHICmdList.UnlockBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI);
	}

	/*{
		uint32 IndexSize = Indices.Num() * sizeof(uint16);
		IndexBuffer.Indices = Indices;
		void* IndexBufferData = RHICmdList.LockBuffer(IndexBuffer.IndexBufferRHI, 0, IndexSize, RLM_WriteOnly);
		FMemory::Memcpy(IndexBufferData, Indices.GetData(), IndexSize);
		RHICmdList.UnlockBuffer(IndexBuffer.IndexBufferRHI);
	}*/
}

void FUnLive2DSceneProxy::FUnLive2DVertexBuffer::GetUnLive2DVertexData(TArray<FDynamicMeshVertex>& OutVertices, TArray<uint16>& OutIndices) const
{
	OutVertices.Empty();
	bool bCreateIndices = OutIndices.Num() > 0;
	for (auto& DrawableIndex : DrawableCounts)
	{

		csmFloat32 Opacity = UnLive2DRawModel->GetDrawableOpacity(DrawableIndex); // 获取不透明度
		if (!bCreateIndices)
		{
			const csmInt32 VertexIndexCount = UnLive2DRawModel->GetDrawableVertexIndexCount(DrawableIndex); // 获得Drawable的顶点索引个数
			const csmUint16* IndicesArray = const_cast<csmUint16*>(UnLive2DRawModel->GetDrawableVertexIndices(DrawableIndex)); //顶点索引
			for (int32 VertexIndex = 0; VertexIndex < VertexIndexCount; ++VertexIndex)
			{
				OutIndices.Add(OutVertices.Num() + IndicesArray[VertexIndex]);
			}
		}

		const csmInt32 NumVertext = UnLive2DRawModel->GetDrawableVertexCount(DrawableIndex); // 获得Drawable顶点的个数
		const csmFloat32* VertexArray = UnLive2DRawModel->GetDrawableVertices(DrawableIndex); // 顶点组
		const  Live2D::Cubism::Core::csmVector2* UVArray = UnLive2DRawModel->GetDrawableVertexUvs(DrawableIndex); // 获取UV组

		FUnLiveVector4 ChanelFlag = FVector4f::One();
		FUnLiveMatrix MartixForDraw;
		FUnLiveVector4 ts_BaseColor;
		bool bMask = RenderState->UnLive2DFillMaskParameter(DrawableIndex, MartixForDraw, ts_BaseColor, ChanelFlag);
		bool bInvertedMesk = UnLive2DRawModel->GetDrawableInvertedMask(DrawableIndex);
		for (int32 VertexIndex = 0; VertexIndex < NumVertext; ++VertexIndex)
		{
			FDynamicMeshVertex& Vert = OutVertices.AddDefaulted_GetRef();
#if WITH_EDITOR
			Vert.Position = FVector3f(VertexArray[VertexIndex * 2] * ScaleMesh, Depth, VertexArray[VertexIndex * 2 + 1] * ScaleMesh);
#else
			Vert.Position = FVector3f(VertexArray[VertexIndex * 2] * ScaleMesh, 0.f, VertexArray[VertexIndex * 2 + 1] * ScaleMesh);
#endif
			Vert.TextureCoordinate[0] = FVector2f(UVArray[VertexIndex].X, 1 - UVArray[VertexIndex].Y);// UE UV坐标与Live2D的Y坐标是相反的

			FVector2f MaskUV = FVector2f::One();
			if (bMask)
			{
				FVector4f Position = FVector4f(VertexArray[VertexIndex * 2], VertexArray[VertexIndex * 2 + 1], 0.f, 1.f);
				FVector4f ClipPosition = MartixForDraw.TransformFVector4(Position);
				MaskUV = FVector2f(ClipPosition.X, 1 + ClipPosition.Y);
				MaskUV /= ClipPosition.W;
			}

			Vert.TextureCoordinate[1] = MaskUV;
			Vert.TextureCoordinate[2] = FVector2f(bMask ? 1.f : 0.f, bInvertedMesk ? 1.f : 0.f);
			Vert.TextureCoordinate[3] = FVector2f(Opacity);
			Vert.Color = FColor(ChanelFlag.X * 255, ChanelFlag.Y * 255, ChanelFlag.Z * 255, ChanelFlag.W * 255);
			Vert.TangentX.Vector.X = 127;
			Vert.TangentX.Vector.W = 127;
			Vert.TangentZ.Vector.Z = 127;
			Vert.TangentZ.Vector.W = 127;
		}

	}
}
