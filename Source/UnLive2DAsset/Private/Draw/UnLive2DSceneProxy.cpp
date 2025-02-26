#include "Draw/UnLive2DSceneProxy.h"
#include "UnLive2DRendererComponent.h"
#include "Stats/Stats2.h"
#include "UnLive2DAssetModule.h"
#include "FWPort/UnLive2DRawModel.h"
#include "FWPort/UnLive2DModelRender.h"
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 1
#include "MaterialDomain.h"
//#include "PrimitiveUniformShaderParametersBuilder.h"
#endif
#include "Materials/MaterialInstanceDynamic.h"
#include "Rendering/StaticLightingSystemInterface.h"
#include "Engine/TextureRenderTarget2D.h"
#include "PhysicsEngine/BodySetup.h"
#include "BodySetupEnums.h"
#include "SceneInterface.h"
#include "Materials/Material.h"

DECLARE_CYCLE_STAT(TEXT("UnLive2DSceneProxy GDME"), STAT_UnLive2DSceneProxy_GetDynamicMeshElements, STATGROUP_UnLive2D);
DECLARE_CYCLE_STAT(TEXT("UnLive2DVertexBuffer GDME"), STAT_UnLive2DVertexBuffer_UpdateSection_RenderThread, STATGROUP_UnLive2D);

#if WITH_EDITOR
TArray<int32> ShowLive2DLevelArr; 

static void Live2DShowLevelIndex(const TArray<FString>& Args, UWorld* InWorld)
{
	ShowLive2DLevelArr.Empty();
	for (const FString& InValue: Args)
	{
		if (InValue.IsNumeric())
		{
			ShowLive2DLevelArr.Add(FCString::Atoi(*InValue));
		}
	}
}

FAutoConsoleCommandWithWorldAndArgs UnLive2DShowLevelIndex(TEXT("Live2DShowLevelIndex"), TEXT("单独显示一个层级"), FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(Live2DShowLevelIndex));

#endif


SIZE_T FUnLive2DSceneProxy::GetTypeHash() const
{
	static size_t UniquePointer;
	return reinterpret_cast<size_t>(&UniquePointer);
}

bool FUnLive2DSceneProxy::CanBeOccluded() const
{
	return !MaterialRelevance.bDisableDepthTest;
}

FUnLive2DSceneProxy::FUnLive2DSceneProxy(UUnLive2DRendererComponent* InComponent)
	: UnLive2DProxyBase(InComponent)
	, bCombinedbBatch(true)
{
	UnLive2DRawModel = InComponent->GetUnLive2DRawModel().Pin();
	if (!UnLive2DRawModel.IsValid()) return;
	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel->GetModel();
	if (UnLive2DModel == nullptr) return;

	if (CreateClippingManager())
	{
		const csmInt32 BufferHeight = UnLive2DClippingManager->GetClippingMaskBufferSize();
		InComponent->InitLive2DRenderData(EUnLive2DRenderType::Mesh, BufferHeight);
	}

	UpdataSections();
}

FUnLive2DSceneProxy::~FUnLive2DSceneProxy()
{
	ClearSections();
	for (auto& Item : UnLive2DToBlendMaterialList)
	{
		if (Item.Value == nullptr) continue;

		Item.Value->RemoveFromRoot();
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 4
		Item.Value->OnRemovedAsOverride(OwnerComponent.Get());
#endif
	}
	UnLive2DToBlendMaterialList.Empty();

}

bool FUnLive2DSceneProxy::OnUpData()
{
	bool bUpdate = false;
	for (int32 i = 0; i < Sections.Num(); i++)
	{
		if (IsCombinedbBatchDidChange(Sections[i]->DrawableCounts))
		{
			Sections[i]->UpdateVertices();
			bUpdate = true;
		}
	}

	return bUpdate;
}

void FUnLive2DSceneProxy::OnUpDataRenderer()
{
	FBox NewLocalBox(ForceInit);
	for (int32 i = 0; i < Sections.Num(); i++)
	{
		NewLocalBox += Sections[i]->GetLocalBox();
	}
	LocalBox = FBoxSphereBounds(NewLocalBox);

	ENQUEUE_RENDER_COMMAND(UnLive2DSceneProxy_OnUpData)([this](FRHICommandListImmediate& RHICmdList)
	{
		ERHIFeatureLevel::Type FeatureLevel = GetScene().GetFeatureLevel();
		if (UnLive2DClippingManager.IsValid())
		{
			bool bNoLowPreciseMask = false;
			UnLive2DClippingManager->SetupClippingContext(bNoLowPreciseMask);
			UnLive2DClippingManager->RenderMask_Full(RHICmdList, FeatureLevel, OwnerComponent->GetMaskTextureRHIRef());
		}
		for (int32 i = 0; i < Sections.Num(); i++)
		{
			Sections[i]->UpdateSection_RenderThread(RHICmdList);
		}
	});
}

void FUnLive2DSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
{
	SCOPE_CYCLE_COUNTER(STAT_UnLive2DSceneProxy_GetDynamicMeshElements);
	if (!UnLive2DRawModel.IsValid()) return;
	const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;
	//Go for each section, creating a batch and collecting it
#if WITH_EDITOR
	bool bDebugLevel = ShowLive2DLevelArr.Num() > 0;
#endif

	for (int32 SectionIndex = 0; SectionIndex < Sections.Num(); SectionIndex++)
	{
#if WITH_EDITOR
		if (bDebugLevel && !ShowLive2DLevelArr.Contains(SectionIndex)) continue;
#endif
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
	// Draw bounds
/*
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		if (VisibilityMap & (1 << ViewIndex))
		{
			if (ViewFamily.EngineShowFlags.Collision && IsCollisionEnabled() && BodySetup->GetCollisionTraceFlag() != ECollisionTraceFlag::CTF_UseComplexAsSimple)
			{
				FTransform GeomTransform(GetLocalToWorld());
				BodySetup->AggGeom.GetAggGeom(GeomTransform, GetSelectionColor(FColor(157, 149, 223, 255), IsSelected(), IsHovered()).ToFColor(true), NULL, false, false, DrawsVelocity(), ViewIndex, Collector);
			}

			// Render bounds
			RenderBounds(Collector.GetPDI(ViewIndex), ViewFamily.EngineShowFlags, GetBounds(), IsSelected());
		}
	}
#endif*/
}


FPrimitiveViewRelevance FUnLive2DSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance Result;
	MaterialRelevance.SetPrimitiveViewRelevance(Result);

	Result.bDrawRelevance = IsShown(View);
	Result.bShadowRelevance = IsShadowCast(View);
	Result.bDynamicRelevance = true;
	Result.bRenderInMainPass = ShouldRenderInMainPass();
	Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
	Result.bRenderCustomDepth = ShouldRenderCustomDepth();
	Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
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

#if WITH_EDITOR
void FUnLive2DSceneProxy::UpDataUnLive2DProperty(FName PropertyName)
{
	if (PropertyName == TEXT("TintColorAndOpacity"))
	{
		for (auto& Item : UnLive2DToBlendMaterialList)
		{
			if (Item.Value == nullptr) continue;
			Item.Value->SetVectorParameterValue("TintColorAndOpacity", OwnerComponent->GetUnLive2D()->TintColorAndOpacity);
		}
	}
}
#endif

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

		bool bHasPrecomputedVolumetricLightmap;
		FMatrix PreviousLocalToWorld;
		int32 SingleCaptureIndex;
		bool bOutputVelocity;
		GetScene().GetPrimitiveUniformShaderParameters_RenderThread(GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, PreviousLocalToWorld, SingleCaptureIndex, bOutputVelocity);
		bOutputVelocity |= AlwaysHasVelocity();
#if ENGINE_MAJOR_VERSION < 5
		DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), PreviousLocalToWorld, GetBounds(), GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap, DrawsVelocity(), bOutputVelocity);
#elif ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 4
		DynamicPrimitiveUniformBuffer.Set(Collector.GetRHICommandList(), GetLocalToWorld(), GetLocalToWorld(), GetBounds(), GetLocalBounds(), false, false, AlwaysHasVelocity());
#else
		DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), PreviousLocalToWorld, GetBounds(), GetLocalBounds(), GetLocalBounds(), ReceivesDecals(), bHasPrecomputedVolumetricLightmap, bOutputVelocity, GetCustomPrimitiveData());

#endif

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
	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel->GetModel();
	if (UnLive2DModel == nullptr) return nullptr;
	Rendering::CubismRenderer::CubismBlendMode BlendMode = UnLive2DModel->GetDrawableBlendMode(DrawableIndex);

	const csmInt32 TextureIndex = UnLive2DModel->GetDrawableTextureIndices(DrawableIndex);
	uint16 BlendModeIndex = BlendMode * 100;
	uint16 MapIndex = BlendModeIndex + TextureIndex;
	UMaterialInstanceDynamic* Material = nullptr;
	
#if ENGINE_MAJOR_VERSION < 5
	UMaterialInstanceDynamic* const* FindMaterial = UnLive2DToBlendMaterialList.Find(MapIndex);
#else
	TObjectPtr<UMaterialInstanceDynamic> const* FindMaterial = UnLive2DToBlendMaterialList.Find(MapIndex);
#endif
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
#if ENGINE_MAJOR_VERSION < 5
		UUnLive2DRendererComponent* RendererComponent = OwnerComponent;
#else
		UUnLive2DRendererComponent* RendererComponent = const_cast<UUnLive2DRendererComponent*>(OwnerComponent.Get());
#endif
		if (RendererComponent->GetUnLive2D() == nullptr || !RendererComponent->GetUnLive2D()->TextureAssets.IsValidIndex(TextureIndex))
			return nullptr;
		Material = UMaterialInstanceDynamic::Create(MaterialInterface, RendererComponent);
		Material->SetTextureParameterValue(TEXT("UnLive2D"), RendererComponent->GetUnLive2D()->TextureAssets[TextureIndex]);
		if (auto MaskBufferRenderTarget = OwnerComponent->GetTextureRenderTarget2D())
		{
			Material->SetTextureParameterValue(TEXT("UnLive2DMask"), MaskBufferRenderTarget);
		}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 4
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

	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel->GetModel();
	// 当前绘制使用数据
	TArray<FUnLive2DSectionData> UnLive2DSectionDataArr;

	// 获取绘制数据
	{
		for (csmInt32 i = 0; i < SortedDrawableIndexList.Num(); i++)
		{
			const csmInt32 DrawableIndex = SortedDrawableIndexList[i];
			if (!GetDrawableDynamicIsVisible(DrawableIndex)) continue;
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

	ERHIFeatureLevel::Type FeatureLevel = GetScene().GetFeatureLevel();
	if (MaterialRelevance.Raw == 0)
	{
		for (auto& Item : UnLive2DToBlendMaterialList)
		{
			if (Item.Value == nullptr) continue;
			MaterialRelevance |= Item.Value->GetRelevance_Concurrent(FeatureLevel);
		}
	}

#if WITH_EDITOR
#if ENGINE_MAJOR_VERSION < 5 || ENGINE_MINOR_VERSION <= 4
	ENQUEUE_RENDER_COMMAND(FMeshRenderBufferSetDestroy)(
		[this, UnLive2DSectionDataArr](FRHICommandListImmediate& RHICmdList)
		{
#endif
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
#if ENGINE_MAJOR_VERSION < 5 || ENGINE_MINOR_VERSION <= 4
		});
#endif
#endif

	// 获取渲染数据
	{
		FBox NewLocalBox(ForceInit);
		// Copy each section
		const int32 NumSections = UnLive2DSectionDataArr.Num();
		Sections.AddZeroed(NumSections);
		for (int32 SectionIdx = 0; SectionIdx < NumSections; SectionIdx++)
		{
			FUnLive2DVertexBuffer* NewSection = new FUnLive2DVertexBuffer(UnLive2DModel, UnLive2DClippingManager.Get(), UnLive2DSectionDataArr[SectionIdx].DrawableCounts, FeatureLevel, Live2DScale,UnLive2DSectionDataArr[SectionIdx].Material);
			Sections[SectionIdx] = NewSection;
#if WITH_EDITOR
			//NewSection->Depth = SectionIdx * 5;
			NewSection->Depth = 0;
#endif
			NewLocalBox += NewSection->GetLocalBox();
			
		}
		LocalBox = FBoxSphereBounds(NewLocalBox);
		ENQUEUE_RENDER_COMMAND(UnLive2DSceneProxy_UpdataSections)(
			[this, FeatureLevel](FRHICommandListImmediate& RHICmdList)
			{
				for (int32 i = 0; i < Sections.Num(); i++)
				{
					Sections[i]->InitRHI(RHICmdList);
				}
			});
	}

	return true;
}

FUnLive2DSceneProxy::FUnLive2DVertexBuffer::FUnLive2DVertexBuffer(Csm::CubismModel* InUnLive2DRawModel, CubismClippingManager_UE* InClippingManager, const TArray<int32>& InDrawableCounts, ERHIFeatureLevel::Type InFeatureLevel, uint8 InScaleMesh,UMaterialInterface* InMaterial)
	: UnLive2DRawModel(InUnLive2DRawModel)
	, RenderState(InClippingManager)
	, ScaleMesh(InScaleMesh)
	, bMarkDirty(false)
	, DrawableCounts(InDrawableCounts)
	, Material(InMaterial)
	, VertexFactory(InFeatureLevel, "FProcMeshProxySection")
#if WITH_EDITOR
	, Depth(0)
#endif
{
	if (Material == nullptr)
	{
		Material = UMaterial::GetDefaultMaterial(MD_Surface);
	}

	TArray<FDynamicMeshVertex> Vertices;
	GetUnLive2DVertexData(Vertices, IndexBuffer.Indices, LocalBox);
	VertexBuffers.InitFromDynamicVertex(&VertexFactory, Vertices, 4);
}

void FUnLive2DSceneProxy::FUnLive2DVertexBuffer::InitRHI(FRHICommandListBase& RHICmdList)
{
	//ReleaseResource();

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 4
	IndexBuffer.InitResource(RHICmdList);
#else
	IndexBuffer.InitResource();
#endif
}

void FUnLive2DSceneProxy::FUnLive2DVertexBuffer::ReleaseResource()
{
	VertexBuffers.PositionVertexBuffer.ReleaseResource();
	VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
	VertexBuffers.ColorVertexBuffer.ReleaseResource();
	IndexBuffer.ReleaseResource();
	VertexFactory.ReleaseResource();
}

void FUnLive2DSceneProxy::FUnLive2DVertexBuffer::UpdateSection_RenderThread(FRHICommandListBase& RHICmdList)
{
	SCOPE_CYCLE_COUNTER(STAT_UnLive2DVertexBuffer_UpdateSection_RenderThread);
	if (!bMarkDirty) return;
	bMarkDirty = false;
	{
		auto& VertexBuffer = VertexBuffers.PositionVertexBuffer;
#if ENGINE_MAJOR_VERSION >= 5
		void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
		RHICmdList.UnlockBuffer(VertexBuffer.VertexBufferRHI);
#else
		void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
		RHIUnlockVertexBuffer(VertexBuffer.VertexBufferRHI);
#endif
	}
	{
		auto& VertexBuffer = VertexBuffers.ColorVertexBuffer;
#if ENGINE_MAJOR_VERSION >= 5
		void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
		RHICmdList.UnlockBuffer(VertexBuffer.VertexBufferRHI);
#else
		void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
		RHIUnlockVertexBuffer(VertexBuffer.VertexBufferRHI);
#endif
	}

	{
		auto& VertexBuffer = VertexBuffers.StaticMeshVertexBuffer;
#if ENGINE_MAJOR_VERSION >= 5
		void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetTangentSize(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTangentData(), VertexBuffer.GetTangentSize());
		RHICmdList.UnlockBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI);
#else
		void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetTangentSize(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTangentData(), VertexBuffer.GetTangentSize());
		RHIUnlockVertexBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI);
#endif
	}

	{
		auto& VertexBuffer = VertexBuffers.StaticMeshVertexBuffer;
#if ENGINE_MAJOR_VERSION >= 5
		void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetTexCoordSize(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTexCoordData(), VertexBuffer.GetTexCoordSize());
		RHICmdList.UnlockBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI);
#else
		void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetTexCoordSize(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTexCoordData(), VertexBuffer.GetTexCoordSize());
		RHIUnlockVertexBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI);
#endif
	}

}

const FBox& FUnLive2DSceneProxy::FUnLive2DVertexBuffer::GetLocalBox() const
{
	return LocalBox;
}

void FUnLive2DSceneProxy::FUnLive2DVertexBuffer::UpdateVertices()
{
	TArray<uint16> Indices;
	TArray<FDynamicMeshVertex> Vertices;
	GetUnLive2DVertexData(Vertices, IndexBuffer.Indices, LocalBox);

	for (int32 i = 0; i < Vertices.Num(); i++)
	{
		const FDynamicMeshVertex& Vertex = Vertices[i];

		VertexBuffers.PositionVertexBuffer.VertexPosition(i) = Vertex.Position;
		VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 0, Vertex.TextureCoordinate[0]);
		VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 1, Vertex.TextureCoordinate[1]);
		VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 2, Vertex.TextureCoordinate[2]);
		VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 3, Vertex.TextureCoordinate[3]);
		VertexBuffers.ColorVertexBuffer.VertexColor(i) = Vertex.Color;
	}
}

void FUnLive2DSceneProxy::FUnLive2DVertexBuffer::GetUnLive2DVertexData(TArray<FDynamicMeshVertex>& OutVertices, TArray<uint16>& OutIndices, FBox& OutLocalBox) const
{
	OutLocalBox = FBox(ForceInit);
	bool bCreateIndices = OutIndices.Num() > 0;
	OutVertices.Empty();
	int32 Counts = 0;
	int32 VerticesIndex = 0;
	for (auto& DrawableIndex : DrawableCounts)
	{
		Counts += DrawableIndex;
		csmFloat32 Opacity = UnLive2DRawModel->GetDrawableOpacity(DrawableIndex); // 获取不透明度
		if (!bCreateIndices)
		{
			const csmInt32 VertexIndexCount = UnLive2DRawModel->GetDrawableVertexIndexCount(DrawableIndex); // 获得Drawable的顶点索引个数
			const csmUint16* IndicesArray = const_cast<csmUint16*>(UnLive2DRawModel->GetDrawableVertexIndices(DrawableIndex)); //顶点索引
			for (int32 VertexIndex = 0; VertexIndex < VertexIndexCount; ++VertexIndex)
			{
				OutIndices.Add(VerticesIndex + IndicesArray[VertexIndex]);
			}
		}

		const csmInt32 NumVertext = UnLive2DRawModel->GetDrawableVertexCount(DrawableIndex); // 获得Drawable顶点的个数
		const csmFloat32* VertexArray = UnLive2DRawModel->GetDrawableVertices(DrawableIndex); // 顶点组
		const  Live2D::Cubism::Core::csmVector2* UVArray = UnLive2DRawModel->GetDrawableVertexUvs(DrawableIndex); // 获取UV组

		FUnLiveMatrix MartixForDraw;
		FUnLiveVector4 ts_BaseColor;
		bool bMask = RenderState->GetFillMaskMartixForMask(DrawableIndex, MartixForDraw, ts_BaseColor);
		const CubismRenderer::CubismTextureColor* ChanelFlagColor = RenderState->GetChannelFlagAsColorByDrawableIndex(DrawableIndex);
		FLinearColor ClipColor = ChanelFlagColor == nullptr ? FLinearColor::White : FLinearColor(ChanelFlagColor->R, ChanelFlagColor->G, ChanelFlagColor->B, ChanelFlagColor->A);
		bool bInvertedMesk = UnLive2DRawModel->GetDrawableInvertedMask(DrawableIndex);
		for (int32 VertexIndex = 0; VertexIndex < NumVertext; ++VertexIndex)
		{
			FDynamicMeshVertex& Vert = OutVertices.AddDefaulted_GetRef();
#if WITH_EDITOR
			Vert.Position = FULVector3f( Depth, -VertexArray[VertexIndex * 2] * ScaleMesh, VertexArray[VertexIndex * 2 + 1] * ScaleMesh);
#else
			Vert.Position = FULVector3f( 0.f, -VertexArray[VertexIndex * 2] * ScaleMesh,VertexArray[VertexIndex * 2 + 1] * ScaleMesh);
#endif
			Vert.TextureCoordinate[0] = FULVector2f(UVArray[VertexIndex].X, 1 - UVArray[VertexIndex].Y);// UE UV坐标与Live2D的Y坐标是相反的
			OutLocalBox += FVector(Vert.Position);
			FULVector2f MaskUV = FULVector2f(1.f,1.f);
			if (bMask)
			{
				FUnLiveVector4 Position = FUnLiveVector4(VertexArray[VertexIndex * 2], VertexArray[VertexIndex * 2 + 1], 0.f, 1.f);
				FUnLiveVector4 ClipPosition = MartixForDraw.TransformFVector4(Position);
				MaskUV = FULVector2f(ClipPosition.X, 1 + ClipPosition.Y);
				MaskUV /= ClipPosition.W;
			}

			Vert.TextureCoordinate[1] = MaskUV;
			Vert.TextureCoordinate[2] = FULVector2f(bMask ? 1.f : 0.f, bInvertedMesk ? 1.f : 0.f);
			Vert.TextureCoordinate[3] = FULVector2f(Opacity);
			Vert.Color = ClipColor.ToFColor(false);
			Vert.TangentX.Vector.X = 127;
			Vert.TangentX.Vector.W = 127;
			Vert.TangentZ.Vector.Z = 127;
			Vert.TangentZ.Vector.W = 127;

		}
		VerticesIndex += NumVertext;
	}

	bMarkDirty = true;
}
