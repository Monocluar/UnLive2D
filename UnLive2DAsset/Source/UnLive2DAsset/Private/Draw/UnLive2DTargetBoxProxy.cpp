#include "Draw/UnLive2DTargetBoxProxy.h"
#include "UnLive2DRendererComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "FWPort/UnLive2DRawModel.h"
#include "FWPort/UnLive2DModelRender.h"
#include "Engine/TextureRenderTarget2D.h"
#include "DynamicMeshBuilder.h"
#include "RHICommandList.h"
#include "GlobalShader.h"
#include "UnLive2DSetting.h"
#include "MaterialDomain.h"
#include "Kismet/KismetSystemLibrary.h"


TGlobalResource<FUnLive2DTargetBoxProxy::FUnLive2DTargetVertexDeclaration> GUnLive2DTargetVertexDeclaration;

TGlobalResource<FUnLive2DTargetBoxProxy::FUnLive2DTargetVertexDeclaration_Mask> GUnLive2DTargetVertexDeclaration_Mask;

class FUnLive2DTargetShaderVS_Base : public FGlobalShader
{
	DECLARE_INLINE_TYPE_LAYOUT(FUnLive2DTargetShaderVS_Base, NonVirtual);

public:
	FUnLive2DTargetShaderVS_Base() {}

	FUnLive2DTargetShaderVS_Base(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
	}
};

template <bool bMask>
class FUnLive2DTargetShaderVS: public FUnLive2DTargetShaderVS_Base
{
	DECLARE_SHADER_TYPE(FUnLive2DTargetShaderVS, Global);

public:

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("UNLIVE2D_TARGET_MASK"), bMask);
	}

	FUnLive2DTargetShaderVS(){}

	FUnLive2DTargetShaderVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FUnLive2DTargetShaderVS_Base(Initializer)
	{ }
};


typedef FUnLive2DTargetShaderVS<true>  FUnLive2DTargetShaderVS_Mask;
typedef FUnLive2DTargetShaderVS<false> FUnLive2DTargetShaderVS_Nor;

IMPLEMENT_SHADER_TYPE(template<>, FUnLive2DTargetShaderVS_Mask, TEXT("/Plugin/UnLive2DAsset/Private/Target/UnLive2DVertexShader.usf"), TEXT("Main_Mask"), SF_Vertex);
IMPLEMENT_SHADER_TYPE(template<>, FUnLive2DTargetShaderVS_Nor, TEXT("/Plugin/UnLive2DAsset/Private/Target/UnLive2DVertexShader.usf"), TEXT("Main_Normal"), SF_Vertex);

class FUnLive2DTargetShaderPS_Base : public FGlobalShader
{
	DECLARE_INLINE_TYPE_LAYOUT(FUnLive2DTargetShaderPS_Base, NonVirtual);
public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}


	FUnLive2DTargetShaderPS_Base(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{

		Texture.Bind(Initializer.ParameterMap, TEXT("InTexture"));
		TextureSampler.Bind(Initializer.ParameterMap, TEXT("InTextureSampler"));
	}
	FUnLive2DTargetShaderPS_Base(){}

	void SetParameters(FRHIBatchedShaderParameters& BatchedParameters, const FTexture* NormalTexture)
	{
		SetTextureParameter(BatchedParameters, Texture, TextureSampler, NormalTexture);
	}
protected:

	LAYOUT_FIELD(FShaderResourceParameter, Texture);
	LAYOUT_FIELD(FShaderResourceParameter, TextureSampler);
};

class FUnLive2DTargetShaderPS_Nor : public FUnLive2DTargetShaderPS_Base
{
	DECLARE_SHADER_TYPE(FUnLive2DTargetShaderPS_Nor, Global);

public:

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment){}

	FUnLive2DTargetShaderPS_Nor(){}
	FUnLive2DTargetShaderPS_Nor(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FUnLive2DTargetShaderPS_Base(Initializer)
	{ }

};


IMPLEMENT_SHADER_TYPE(, FUnLive2DTargetShaderPS_Nor, TEXT("/Plugin/UnLive2DAsset/Private/Target/UnLive2DPixelShader.usf"), TEXT("Main_Seq_Normal"), SF_Pixel);

template <bool bInvertMask>
class FUnLive2DTargetShaderPS_MaskBase : public FUnLive2DTargetShaderPS_Base
{
	DECLARE_SHADER_TYPE(FUnLive2DTargetShaderPS_MaskBase, Global);

public:
	FUnLive2DTargetShaderPS_MaskBase(){}

	FUnLive2DTargetShaderPS_MaskBase(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FUnLive2DTargetShaderPS_Base(Initializer)
	{
		MaskTexture.Bind(Initializer.ParameterMap, TEXT("InMaskTexture"));
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("UNLIVE2D_TARGET_INVERT_MASK"), bInvertMask);
	}


	void SetParameters(FRHIBatchedShaderParameters& BatchedParameters, const FTexture* NormalTexture, FTextureRHIRef MaskTextureRef)
	{
		SetTextureParameter(BatchedParameters, Texture, TextureSampler, NormalTexture);
		SetTextureParameter(BatchedParameters, MaskTexture, MaskTextureRef);
	}


private:
	LAYOUT_FIELD(FShaderResourceParameter, MaskTexture);
};

typedef FUnLive2DTargetShaderPS_MaskBase<true> FUnLive2DTargetShaderPS_Seq_InvertMask;
typedef FUnLive2DTargetShaderPS_MaskBase<false> FUnLive2DTargetShaderPS_Seq_Mask;

IMPLEMENT_SHADER_TYPE(template<>, FUnLive2DTargetShaderPS_Seq_InvertMask, TEXT("/Plugin/UnLive2DAsset/Private/Target/UnLive2DPixelShader.usf"), TEXT("Main_Seq_Mask"), SF_Pixel);
IMPLEMENT_SHADER_TYPE(template<>, FUnLive2DTargetShaderPS_Seq_Mask, TEXT("/Plugin/UnLive2DAsset/Private/Target/UnLive2DPixelShader.usf"), TEXT("Main_Seq_Mask"), SF_Pixel);


void FUnLive2DTargetBoxProxy::UpdateSection_RenderThread(FRHICommandListImmediate& RHICmdList)
{
	if (!UnLive2DRawModel.IsValid()) return;
	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel.Pin()->GetModel();
	if (UnLive2DModel == nullptr) return;

	const bool IsUsingMask = UnLive2DModel->IsUsingMasking();
	if (IsUsingMask && !UnLive2DClippingManager.IsValid())
	{
		UnLive2DClippingManager = MakeUnique<CubismClippingManager_UE>();

		UnLive2DClippingManager->Initialize(
			UnLive2DModel,
			UnLive2DModel->GetDrawableCount(),
			UnLive2DModel->GetDrawableMasks(),
			UnLive2DModel->GetDrawableMaskCounts()
		);

		ETextureCreateFlags Flags = ETextureCreateFlags(TexCreate_None | TexCreate_RenderTargetable | TexCreate_ShaderResource);
		const csmInt32 BufferHeight = UnLive2DClippingManager->GetClippingMaskBufferSize();
		const FRHITextureCreateDesc Desc = FRHITextureCreateDesc::Create2D(TEXT("FUnLive2DTargetBoxProxy_UpdateSection_RenderThread"), BufferHeight, BufferHeight, PF_R8G8B8A8)
			.SetFlags(Flags).SetClearValue(FClearValueBinding(FLinearColor::White));

		MaskBuffer = RHICreateTexture(Desc);
	}
	UpdataSections(RHICmdList);

	if (RenderTarget)
	{
		DrawSeparateToRenderTarget_RenderThread(RHICmdList, RenderTarget->GetRenderTargetResource());
	}
}

void FUnLive2DTargetBoxProxy::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials /*= false*/) const
{
	OutMaterials.Add(MaterialInstance);
}

void FUnLive2DTargetBoxProxy::OnUpData()
{
	ENQUEUE_RENDER_COMMAND(UnLive2DTargetBoxProxy_OnUpData)(
	[this](FRHICommandListImmediate& RHICmdList)
		{
			UpdateSection_RenderThread(RHICmdList);
		});
}

SIZE_T FUnLive2DTargetBoxProxy::GetTypeHash() const
{
	static size_t UniquePointer;
	return reinterpret_cast<size_t>(&UniquePointer);
}

uint32 FUnLive2DTargetBoxProxy::GetMemoryFootprint() const
{
	return sizeof(*this) + FPrimitiveSceneProxy::GetAllocatedSize();
}

void FUnLive2DTargetBoxProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
{
	if (MaterialInstance == nullptr || RenderTarget == nullptr) return;
	FMaterialRenderProxy* ParentMaterialProxy = MaterialInstance->GetRenderProxy();

	const FMatrix& ViewportLocalToWorld = GetLocalToWorld();

	FMatrix PreviousLocalToWorld;

	if (!GetScene().GetPreviousLocalToWorld(GetPrimitiveSceneInfo(), PreviousLocalToWorld))
	{
		PreviousLocalToWorld = GetLocalToWorld();
	}

	FTextureResource* TextureResource = RenderTarget->GetResource();
	if (TextureResource)
	{
		float U = -RenderTarget->SizeX * static_cast<float>(Pivot.X);
		float V = -RenderTarget->SizeY * static_cast<float>(Pivot.Y);
		float UL = RenderTarget->SizeX * (1.0f - static_cast<float>(Pivot.X));
		float VL = RenderTarget->SizeY * (1.0f - static_cast<float>(Pivot.Y));

		int32 VertexIndices[4];

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			FDynamicMeshBuilder MeshBuilder(Views[ViewIndex]->GetFeatureLevel());

			if (VisibilityMap & (1 << ViewIndex))
			{
				VertexIndices[0] = MeshBuilder.AddVertex(-FVector3f(U,  0, V), FVector2f(0, 0), FVector3f(0, 1, 0), FVector3f(0, 0, 1), FVector3f(-1, 0, 0), FColor::White);
				VertexIndices[1] = MeshBuilder.AddVertex(-FVector3f(U,  0, VL), FVector2f(0, 1), FVector3f(0, 1, 0), FVector3f(0, 0, 1), FVector3f(-1, 0, 0), FColor::White);
				VertexIndices[2] = MeshBuilder.AddVertex(-FVector3f(UL, 0, VL), FVector2f(1, 1), FVector3f(0, 1, 0), FVector3f(0, 0, 1), FVector3f(-1, 0, 0), FColor::White);
				VertexIndices[3] = MeshBuilder.AddVertex(-FVector3f(UL, 0, V), FVector2f(1, 0), FVector3f(0, 1, 0), FVector3f(0, 0,-1), FVector3f(-1, 0, 0), FColor::White);

				MeshBuilder.AddTriangle(VertexIndices[0], VertexIndices[2], VertexIndices[1]);
				MeshBuilder.AddTriangle(VertexIndices[0], VertexIndices[3], VertexIndices[2]);

				FDynamicMeshBuilderSettings Settings;
				Settings.bDisableBackfaceCulling = false;
				Settings.bReceivesDecals = true;
				Settings.bUseSelectionOutline = true;
				MeshBuilder.GetMesh(ViewportLocalToWorld, PreviousLocalToWorld, ParentMaterialProxy, SDPG_World, Settings, nullptr, ViewIndex, Collector, FHitProxyId());
			}
		}
	}
}

FPrimitiveViewRelevance FUnLive2DTargetBoxProxy::GetViewRelevance(const FSceneView* View) const
{

	FPrimitiveViewRelevance Result;

	//MaterialRelevance.SetPrimitiveViewRelevance(Result);

	Result.bDrawRelevance = IsShown(View) && View->Family->EngineShowFlags.Paper2DSprites; // 使用Sprites的
	Result.bDynamicRelevance = true;
	Result.bRenderCustomDepth = ShouldRenderCustomDepth();
	Result.bRenderInMainPass = ShouldRenderInMainPass();
	Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
	Result.bShadowRelevance = IsShadowCast(View);
	Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
	Result.bEditorPrimitiveRelevance = false;
	Result.bVelocityRelevance = DrawsVelocity() && Result.bOpaque && Result.bRenderInMainPass;

	return Result;
}

bool FUnLive2DTargetBoxProxy::UpdataSections(FRHICommandListImmediate& RHICmdList)
{
	TArray<uint16> SortedDrawableIndexList;
	if (!UpDataDrawableIndexList(SortedDrawableIndexList)) return false;


	UnLive2DSectionDataArr.Empty();
	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel.Pin()->GetModel();
	for (const uint16& DrawableIndex : SortedDrawableIndexList)
	{
		// <Drawable如果不是显示状态，则通过处理
		if (!UnLive2DModel->GetDrawableDynamicFlagIsVisible(DrawableIndex)) continue;

		if (0 == UnLive2DModel->GetDrawableVertexIndexCount(DrawableIndex)) continue;

		csmFloat32 Opacity = UnLive2DModel->GetDrawableOpacity(DrawableIndex); // 获取不透明度
		if (Opacity <= 0.f) continue;

		const csmInt32 TextureIndex = UnLive2DModel->GetDrawableTextureIndices(DrawableIndex);

		uint8 Flags = GetUnLive2DShaderFlagsByDrawableIndex(DrawableIndex, UnLive2DModel);
		FUnLive2DSectionData* MeshSectionData = nullptr;
		if (!bCombinedbBatch || UnLive2DSectionDataArr.Num() == 0 || UnLive2DSectionDataArr.Top().Flags != Flags || UnLive2DSectionDataArr.Top().TextureIndex != TextureIndex)
		{
			MeshSectionData = &UnLive2DSectionDataArr.AddDefaulted_GetRef();
			MeshSectionData->Flags = Flags;
			MeshSectionData->TextureIndex = TextureIndex;
			MeshSectionData->CacheIndexCount = 0;
		}
		else
			MeshSectionData = &UnLive2DSectionDataArr.Top();

		MeshSectionData->DrawableCounts.Add(DrawableIndex);
	}

	return true;
}

uint8 FUnLive2DTargetBoxProxy::GetUnLive2DShaderFlagsByDrawableIndex(const uint16& InDrawableIndex, Live2D::Cubism::Framework::CubismModel* InUnLive2DModel) const
{
	uint8 Flags = EUnLive2DShaderFlags::BlendMode_Nor;
	Rendering::CubismRenderer::CubismBlendMode BlendMode = InUnLive2DModel->GetDrawableBlendMode(InDrawableIndex);
	switch (BlendMode)
	{
	case Rendering::CubismRenderer::CubismBlendMode_Additive:
		Flags = EUnLive2DShaderFlags::BlendMode_Add;
		break;
	case Rendering::CubismRenderer::CubismBlendMode_Multiplicative:
		Flags = EUnLive2DShaderFlags::BlendMode_Mul;
		break;
	}
	if (UnLive2DClippingManager.IsValid() && UnLive2DClippingManager->GetClipContextInDrawableIndex(InDrawableIndex))
	{
		Flags |= EUnLive2DShaderFlags::Mesk;
		if (InUnLive2DModel->GetDrawableInvertedMask(InDrawableIndex))
		{
			Flags |= EUnLive2DShaderFlags::InvertedMesk;
		}
	}

	return Flags;
}

const UTexture2D* FUnLive2DTargetBoxProxy::GetTexture(const uint8& TextureIndex) const
{
	if (OwnerComponent == nullptr || OwnerComponent->GetUnLive2D() == nullptr || !OwnerComponent->GetUnLive2D()->TextureAssets.IsValidIndex(TextureIndex)) return nullptr;

	return OwnerComponent->GetUnLive2D()->TextureAssets[TextureIndex];
}

bool FUnLive2DTargetBoxProxy::IsCombinedbBatchDidChange(const Live2D::Cubism::Framework::CubismModel* UnLive2DModel, const FUnLive2DSectionData& InSectionData) const
{
	if (!bCombinedbBatch) return true;

	for (const int32& DrawableIndex : InSectionData.DrawableCounts)
	{
		if (UnLive2DModel->GetDrawableDynamicFlagVertexPositionsDidChange(DrawableIndex)) return true;
	}

	return false;
}

template<typename T>
constexpr uint16 FUnLive2DTargetBoxProxy::GetVertexBySectionData(FRHICommandListImmediate& RHICmdList, Live2D::Cubism::Framework::CubismModel* UnLive2DModel, FUnLive2DSectionData& SectionData)
{
	TArray<T> OutVertexs;
	bool bMask = SectionData.Flags & EUnLive2DShaderFlags::Mesk;
	TArray<uint16> OutIndices;
	uint16 VerticesIndex = 0;
	bool bCreateIndexBuffers = SectionData.CacheIndexBuffersRHI.IsValid();
	if (bCreateIndexBuffers && !IsCombinedbBatchDidChange(UnLive2DModel, SectionData)) return SectionData.CacheVerticesIndexCount;

	for (const int32& DrawableIndex : SectionData.DrawableCounts)
	{
		if (!bCombinedbBatch && bCreateIndexBuffers && !UnLive2DModel->GetDrawableDynamicFlagVertexPositionsDidChange(DrawableIndex)) // 顶点位置没有变化就不需要更新Buffer
			continue;
		if (!bCreateIndexBuffers)
		{
			const csmInt32 VertexIndexCount = UnLive2DModel->GetDrawableVertexIndexCount(DrawableIndex); // 获得Drawable的顶点索引个数
			const csmUint16* IndicesArray = UnLive2DModel->GetDrawableVertexIndices(DrawableIndex); //顶点索引

			for (int32 Index = 0; Index < VertexIndexCount; ++Index)
			{
				OutIndices.Add(VerticesIndex + IndicesArray[Index]);
			}
			SectionData.CacheIndexCount += VertexIndexCount;
		}
		csmFloat32 Opacity = UnLive2DModel->GetDrawableOpacity(DrawableIndex); // 获取不透明度

		const csmInt32 NumVertext = UnLive2DModel->GetDrawableVertexCount(DrawableIndex); // 获得Drawable顶点的个数
		const csmFloat32* VertexArray = UnLive2DModel->GetDrawableVertices(DrawableIndex); // 顶点组
		const  Live2D::Cubism::Core::csmVector2* UVArray = UnLive2DModel->GetDrawableVertexUvs(DrawableIndex); // 获取UV组

		CubismRenderer::CubismTextureColor* ChanelFlagColor = nullptr;
		FUnLiveMatrix MartixForMask;
		FLinearColor BaseColor = FLinearColor(1.f, 1.f, 1.f, Opacity);
		FVector4f MaskPos;
		if (bMask)
		{
			ChanelFlagColor = UnLive2DClippingManager->GetChannelFlagAsColorByDrawableIndex(DrawableIndex);
			UnLive2DClippingManager->GetFillMaskMartixForMask(DrawableIndex, MartixForMask, MaskPos);
		}
		for (int32 VertexIndex = 0; VertexIndex < NumVertext; ++VertexIndex)
		{
			T& Vert = OutVertexs.AddDefaulted_GetRef();
			Vert.Position = FVector2f(VertexArray[VertexIndex * 2], VertexArray[VertexIndex * 2 + 1]);
			Vert.UV = FVector2f(UVArray[VertexIndex].X, UVArray[VertexIndex].Y);
			Vert.Color = BaseColor;
			//UKismetSystemLibrary::DrawDebugPoint(GetScene().GetWorld(), FVector(Vert.Position.X, 0.f,Vert.Position.Y), 2.f, FLinearColor::Red);
			if constexpr (std::is_base_of<FUnLive2DRenderTargetVertex_Mask, T>())
			{
				FVector4f Position = FVector4f(Vert.Position.X, Vert.Position.Y, 0.f, 1.f);
				Vert.ClipPosition = MartixForMask.TransformFVector4(Position);
					
				if (ChanelFlagColor)
					Vert.ChanelFlag = FLinearColor(ChanelFlagColor->R, ChanelFlagColor->G, ChanelFlagColor->B, ChanelFlagColor->A);

			}

		}
		VerticesIndex += NumVertext;
	}

	// UpDataBuffers
	{
		if (OutIndices.Num() > 0)
		{
			const uint32 IndexSize = SectionData.CacheIndexCount * sizeof(uint16);
			FRHIResourceCreateInfo CreateInfo(TEXT("UnLive2DCacheIndexBuffer"));
			SectionData.CacheIndexBuffersRHI = RHICmdList.CreateIndexBuffer(sizeof(uint16), IndexSize, BUF_Static, CreateInfo);
			void* IndexBufferData = RHICmdList.LockBuffer(SectionData.CacheIndexBuffersRHI, 0, IndexSize, RLM_WriteOnly);
			FMemory::Memcpy(IndexBufferData, OutIndices.GetData(), IndexSize);
			RHICmdList.UnlockBuffer(SectionData.CacheIndexBuffersRHI);
		}
		const uint32 VertexSize = VerticesIndex * sizeof(T);
		if (!SectionData.CacheVertexBufferRHI.IsValid())
		{
			FRHIResourceCreateInfo CreateInfo(TEXT("UnLive2DCacheVertexBufferRHI"));
			SectionData.CacheVertexBufferRHI = RHICmdList.CreateVertexBuffer(VertexSize, BUF_Dynamic, CreateInfo);
			SectionData.CacheVerticesIndexCount = VerticesIndex;
		}
		void* VertexBufferData = RHICmdList.LockBuffer(SectionData.CacheVertexBufferRHI, 0, VertexSize, RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, OutVertexs.GetData(), VertexSize);
		RHICmdList.UnlockBuffer(SectionData.CacheVertexBufferRHI);
	}
	return VerticesIndex;
}

void FUnLive2DTargetBoxProxy::DrawSeparateToRenderTarget_RenderThread(FRHICommandListImmediate& RHICmdList, FTextureRenderTargetResource* OutTextureRenderTargetResource)
{
	if (OutTextureRenderTargetResource == nullptr || !UnLive2DRawModel.IsValid()) return;

	ERHIFeatureLevel::Type FeatureLevel = GetScene().GetFeatureLevel();
	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel.Pin()->GetModel();
	if (UnLive2DClippingManager.IsValid())
	{
		UnLive2DClippingManager->SetupClippingContext(UnLive2DModel);
		// 先绘制遮罩Buffer
		//RenderMask_Full(RHICmdList, UnLive2DModel);
		UnLive2DClippingManager->RenderMask_Full(RHICmdList, UnLive2DModel, FeatureLevel, MaskBuffer);
	}
	return;
#if ENGINE_MAJOR_VERSION >=5 && ENGINE_MINOR_VERSION > 1
	const FTextureRHIRef RenderTargetTexture = OutTextureRenderTargetResource->GetRenderTargetTexture();
#else
	FRHITexture2D* RenderTargetTexture = OutTextureRenderTargetResource->GetRenderTargetTexture();
#endif
	// DrawClear
	{
		RHICmdList.Transition(FRHITransitionInfo(RenderTargetTexture, ERHIAccess::WritableMask));

		FRHIRenderPassInfo RPInfo(RenderTargetTexture, ERenderTargetActions::Clear_Store, OutTextureRenderTargetResource->TextureRHI);
		RHICmdList.BeginRenderPass(RPInfo, TEXT("DrawClear"));
		RHICmdList.EndRenderPass();
	}
	for (FUnLive2DSectionData& SectionData : UnLive2DSectionDataArr)
	{

		bool bMask = SectionData.Flags & EUnLive2DShaderFlags::Mesk;
		uint16 NumVertext = 0;
		if (bMask)
			NumVertext = GetVertexBySectionData<FUnLive2DRenderTargetVertex_Mask>(RHICmdList, UnLive2DModel, SectionData);
		else
			NumVertext = GetVertexBySectionData<FUnLive2DRenderTargetVertex>(RHICmdList, UnLive2DModel, SectionData);
		if (!bMask) continue;
		// DrawSeparate
		{
			RHICmdList.Transition(FRHITransitionInfo(RenderTargetTexture, ERHIAccess::WritableMask));

			FRHIRenderPassInfo RPInfo(RenderTargetTexture, ERenderTargetActions::Load_Store, OutTextureRenderTargetResource->TextureRHI);
			RHICmdList.BeginRenderPass(RPInfo, TEXT("DrawSeparate"));
			{
				// Update viewport.
				RHICmdList.SetViewport(
					0.f, 0.f, 0.f,
					OutTextureRenderTargetResource->GetSizeX(), OutTextureRenderTargetResource->GetSizeY(), 1.f);

				FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);

				// Set the graphic pipeline state.
				FGraphicsPipelineStateInitializer GraphicsPSOInit;
				RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
				GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
				GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 4
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, ERasterizerDepthClipMode::DepthClip, true>::GetRHI();
#else
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, true, true>::GetRHI();
#endif
				GraphicsPSOInit.PrimitiveType = PT_TriangleList;

				if (SectionData.Flags & EUnLive2DShaderFlags::BlendMode_Nor)
					GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_InverseSourceAlpha, BO_Add, BF_One, BF_InverseSourceAlpha>::GetRHI();
					//GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
				else if (SectionData.Flags & EUnLive2DShaderFlags::BlendMode_Add)
					GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_One, BO_Add, BF_One, BF_One>::GetRHI();
				else
					GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_DestColor, BF_InverseSourceAlpha, BO_Add, BF_Zero, BF_One>::GetRHI();


				const UTexture2D* Texture = GetTexture(SectionData.TextureIndex);
#if ENGINE_MAJOR_VERSION < 5
				const FTexture* TextureResource = Texture->Resource;
#else
				const FTexture* TextureResource = Texture->GetResource();
#endif
				if (bMask)
				{
					GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GUnLive2DTargetVertexDeclaration_Mask.VertexDeclarationRHI;
					TShaderMapRef<FUnLive2DTargetShaderVS_Mask> VertexShader(GlobalShaderMap);
					GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
					if (SectionData.Flags & EUnLive2DShaderFlags::InvertedMesk)
					{
						TShaderMapRef<FUnLive2DTargetShaderPS_Seq_InvertMask>PixelShader(GlobalShaderMap);
						GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
						SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);
						SetShaderParametersLegacyPS(RHICmdList, PixelShader, TextureResource, MaskBuffer->GetTexture2D());
					}
					else 
					{
						TShaderMapRef<FUnLive2DTargetShaderPS_Seq_Mask>PixelShader(GlobalShaderMap);
						GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
						SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);
						SetShaderParametersLegacyPS(RHICmdList, PixelShader, TextureResource, MaskBuffer->GetTexture2D());
					}
				}
				else 
				{
					GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GUnLive2DTargetVertexDeclaration.VertexDeclarationRHI;
					TShaderMapRef<FUnLive2DTargetShaderPS_Nor>PixelShader(GlobalShaderMap);
					TShaderMapRef<FUnLive2DTargetShaderVS_Nor> VertexShader(GlobalShaderMap);
					GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
					GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
					SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);
					SetShaderParametersLegacyPS(RHICmdList, PixelShader, TextureResource);
				}

				RHICmdList.SetStreamSource(0, SectionData.CacheVertexBufferRHI, 0);
				RHICmdList.DrawIndexedPrimitive(
					SectionData.CacheIndexBuffersRHI,
					/*BaseVertexIndex=*/ 0,
					/*MinIndex=*/ 0,
					/*NumVertices=*/ NumVertext,
					/*StartIndex=*/ 0,
					/*NumPrimitives=*/ SectionData.CacheIndexCount / 3,
					/*NumInstances=*/ 1

				);

			}
			RHICmdList.EndRenderPass();
		}
	}
}

FUnLive2DTargetBoxProxy::FUnLive2DTargetBoxProxy(UUnLive2DRendererComponent* InComponent)
	: UnLive2DProxyBase(InComponent)
	, Pivot(FVector2D(0.5,1))
	, bCombinedbBatch(false)
	, BoundsScale(InComponent->BoundsScale)
{
	if (!UnLive2DRawModel.IsValid()) return;
	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel.Pin()->GetModel();
	if (UnLive2DModel == nullptr) return;

	csmFloat32 CanvasWidth = UnLive2DModel->GetCanvasWidth();
	csmFloat32 CanvasHeight = UnLive2DModel->GetCanvasHeight();

	RenderTarget = NewObject<UTextureRenderTarget2D>(GetTransientPackage());
	RenderTarget->ClearColor = FLinearColor(0.f, 0.f, 0.f, 0.f);
	const EPixelFormat RequestedFormat = FSlateApplication::Get().GetRenderer()->GetSlateRecommendedColorFormat();
	RenderTarget->InitCustomFormat(BoundsScale, BoundsScale, RequestedFormat, false);
	RenderTarget->AddToRoot();

	const UUnLive2DSetting* Setting = GetDefault<UUnLive2DSetting>();
	// 使用3DUI的
	if (Setting->DefaultUnLive2DRenderTargetMaterial.IsNull()) return;
	UMaterialInterface* MaterialInterface = Cast<UMaterialInterface>(Setting->DefaultUnLive2DRenderTargetMaterial.TryLoad());
	if (MaterialInterface == nullptr) return;
	
	UMaterialInstanceDynamic* MaterialDynamic = UMaterialInstanceDynamic::Create(MaterialInterface, InComponent);
	if (MaterialDynamic)
	{
		MaterialDynamic->AddToCluster(InComponent);
		MaterialDynamic->SetTextureParameterValue("SlateUI", RenderTarget);
#if WITH_EDITOR
		TArray<UMaterialInterface*> InUsedMaterialsForVerification;
		InUsedMaterialsForVerification.Add(MaterialDynamic);
		SetUsedMaterialForVerification(InUsedMaterialsForVerification);
#endif
		MaterialDynamic->SetVectorParameterValue("TintColorAndOpacity", InComponent->GetUnLive2D()->TintColorAndOpacity);
		//MaterialDynamic->SetScalarParameterValue("OpacityFromTexture", OpacityFromTexture);
		MaterialInstance = MaterialDynamic;
		MaterialInstance->AddToRoot();
	}

	//MaterialInstance = UMaterial::GetDefaultMaterial(MD_Surface);
}

FUnLive2DTargetBoxProxy::~FUnLive2DTargetBoxProxy()
{
	if (RenderTarget)
	{
		RenderTarget->RemoveFromRoot();
	}
	UnLive2DClippingManager.Reset();
	if (MaterialInstance)
	{
		MaterialInstance->RemoveFromRoot();
	}
	MaterialInstance = nullptr;
}

FUnLive2DTargetBoxProxy::FUnLive2DSectionData::~FUnLive2DSectionData()
{
	CacheIndexBuffersRHI.SafeRelease();
	CacheVertexBufferRHI.SafeRelease();
}

void FUnLive2DTargetBoxProxy::FUnLive2DTargetVertexDeclaration::InitRHI(FRHICommandListBase& RHICmdList)
{
	FVertexDeclarationElementList Elements;
	uint16 Stride = sizeof(FUnLive2DRenderTargetVertex);
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUnLive2DRenderTargetVertex, Position), VET_Float2, 0, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUnLive2DRenderTargetVertex, UV), VET_Float2, 1, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUnLive2DRenderTargetVertex, Color), VET_Float4, 2, Stride));

	VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
}

void FUnLive2DTargetBoxProxy::FUnLive2DTargetVertexDeclaration::ReleaseRHI()
{
	VertexDeclarationRHI.SafeRelease();
}

void FUnLive2DTargetBoxProxy::FUnLive2DTargetVertexDeclaration_Mask::InitRHI(FRHICommandListBase& RHICmdList)
{
	FVertexDeclarationElementList Elements;
	uint16 Stride = sizeof(FUnLive2DRenderTargetVertex_Mask);
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUnLive2DRenderTargetVertex_Mask, Position), VET_Float2, 0, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUnLive2DRenderTargetVertex_Mask, UV), VET_Float2, 1, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUnLive2DRenderTargetVertex_Mask, Color), VET_Float4, 2, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUnLive2DRenderTargetVertex_Mask, ClipPosition), VET_Float4, 3, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUnLive2DRenderTargetVertex_Mask, ChanelFlag), VET_Float4, 4, Stride));

	VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);

}

void FUnLive2DTargetBoxProxy::FUnLive2DTargetVertexDeclaration_Mask::ReleaseRHI()
{
	VertexDeclarationRHI.SafeRelease();
}
