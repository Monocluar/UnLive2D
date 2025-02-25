#include "Draw/IUnLive2DRenderBase.h"
#include "FWPort/UnLive2DRawModel.h"
#include "FWPort/UnLive2DModelRender.h"
#include "DynamicMeshBuilder.h"
#include "RHICommandList.h"
#include "GlobalShader.h"
#include "UnLive2DSetting.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "MaterialDomain.h"
#endif
#include "Kismet/KismetSystemLibrary.h"
#include "ShaderParameterUtils.h"

TGlobalResource<IUnLive2DRTRender::FUnLive2DTargetVertexDeclaration> GUnLive2DTargetVertexDeclaration;

class FUnLive2DTargetShaderVS_Base : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FUnLive2DTargetShaderVS_Base, Global);

public:

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		//return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		return true;
	}
	FUnLive2DTargetShaderVS_Base() {}

	FUnLive2DTargetShaderVS_Base(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
	}
};

IMPLEMENT_SHADER_TYPE(, FUnLive2DTargetShaderVS_Base, TEXT("/Plugin/UnLive2DAsset/Private/Target/UnLive2DVertexShader.usf"), TEXT("Main_Normal"), SF_Vertex);

class FUnLive2DTargetShaderPS_Base : public FGlobalShader
{
	DECLARE_INLINE_TYPE_LAYOUT(FUnLive2DTargetShaderPS_Base, NonVirtual);
public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		//return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		return true;
	}


	FUnLive2DTargetShaderPS_Base(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{

		Texture.Bind(Initializer.ParameterMap, TEXT("InTexture"));
		TextureSampler.Bind(Initializer.ParameterMap, TEXT("InTextureSampler"));
	}
	FUnLive2DTargetShaderPS_Base() {}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
	void SetParameters(FRHIBatchedShaderParameters& BatchedParameters, const FTexture* NormalTexture)
	{
		SetTextureParameter(BatchedParameters, Texture, TextureSampler, NormalTexture);
	}
#else
	template<typename TShaderRHIParamRef>
	void SetParameters(FRHICommandListImmediate& RHICmdList, const TShaderRHIParamRef ShaderRHI, const FTexture* NormalTexture)
	{
		SetTextureParameter(RHICmdList, ShaderRHI, Texture, TextureSampler, NormalTexture);
		//SetSamplerParameter(RHICmdList, ShaderRHI, TextureSampler, TStaticSamplerState<SF_Trilinear, AM_Wrap, AM_Wrap, AM_Wrap>::GetRHI());
	}

#endif

protected:

	LAYOUT_FIELD(FShaderResourceParameter, Texture);
	LAYOUT_FIELD(FShaderResourceParameter, TextureSampler);
};

class FUnLive2DTargetShaderPS_Nor : public FUnLive2DTargetShaderPS_Base
{
	DECLARE_SHADER_TYPE(FUnLive2DTargetShaderPS_Nor, Global);

public:

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment) 
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}

	FUnLive2DTargetShaderPS_Nor() {}
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
	FUnLive2DTargetShaderPS_MaskBase() {}

	FUnLive2DTargetShaderPS_MaskBase(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FUnLive2DTargetShaderPS_Base(Initializer)
	{
		ChannelFlag.Bind(Initializer.ParameterMap, TEXT("InChannelFlag"));
		ClipMatrix.Bind(Initializer.ParameterMap, TEXT("InClipMatrix"));
		MaskTexture.Bind(Initializer.ParameterMap, TEXT("InMaskTexture"));
		MaskTextureSampler.Bind(Initializer.ParameterMap, TEXT("InMaskTextureSampler"));
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FUnLive2DTargetShaderPS_Nor::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("UNLIVE2D_TARGET_INVERT_MASK"), bInvertMask);
	}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
	void SetParameters(FRHIBatchedShaderParameters& BatchedParameters, const FTexture* NormalTexture, FTextureRHIRef MaskTextureRef, const FUnLiveMatrix& InMartixForMask, const FLinearColor& InClipColor)
	{
		FUnLive2DTargetShaderPS_Base::SetParameters(BatchedParameters, NormalTexture);
		SetShaderValue(BatchedParameters, ChannelFlag, InClipColor);
		SetShaderValue(BatchedParameters, ClipMatrix, InMartixForMask);
		//SetTextureParameter(BatchedParameters, Texture, TextureSampler, NormalTexture);
		//SetTextureParameter(BatchedParameters, MaskTexture, MaskTextureRef);
		SetTextureParameter(RHICmdList, ShaderRHI, MaskTexture, MaskTextureSampler, TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(), MaskTextureRef);
	}
#else
	template<typename TShaderRHIParamRef>
	void SetParameters(FRHICommandListImmediate& RHICmdList, const TShaderRHIParamRef ShaderRHI, const FTexture* NormalTexture, FTextureRHIRef MaskTextureRef, const FUnLiveMatrix& InMartixForMask, const FLinearColor& InClipColor)
	{
		FUnLive2DTargetShaderPS_Base::SetParameters(RHICmdList, ShaderRHI, NormalTexture);
		SetShaderValue(RHICmdList, ShaderRHI, ChannelFlag, InClipColor);
		SetShaderValue(RHICmdList, ShaderRHI, ClipMatrix, InMartixForMask);
		//SetTextureParameter(RHICmdList, ShaderRHI, Texture, NormalTexture->TextureRHI);
		SetTextureParameter(RHICmdList, ShaderRHI, MaskTexture, MaskTextureSampler, TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(), MaskTextureRef);
}

#endif

private:
	LAYOUT_FIELD(FShaderParameter, ChannelFlag);
	LAYOUT_FIELD(FShaderParameter, ClipMatrix);
	LAYOUT_FIELD(FShaderResourceParameter, MaskTexture);
	LAYOUT_FIELD(FShaderResourceParameter, MaskTextureSampler);
};

typedef FUnLive2DTargetShaderPS_MaskBase<true> FUnLive2DTargetShaderPS_Seq_InvertMask;
typedef FUnLive2DTargetShaderPS_MaskBase<false> FUnLive2DTargetShaderPS_Seq_Mask;

IMPLEMENT_SHADER_TYPE(template<>, FUnLive2DTargetShaderPS_Seq_InvertMask, TEXT("/Plugin/UnLive2DAsset/Private/Target/UnLive2DPixelShader.usf"), TEXT("Main_Seq_Mask"), SF_Pixel);
IMPLEMENT_SHADER_TYPE(template<>, FUnLive2DTargetShaderPS_Seq_Mask, TEXT("/Plugin/UnLive2DAsset/Private/Target/UnLive2DPixelShader.usf"), TEXT("Main_Seq_Mask"), SF_Pixel);


bool IUnLive2DRenderBase::UpDataDrawableIndexList(TArray<uint16>& OutSortedDrawableIndexList)
{
	if (!UnLive2DRawModel.IsValid()) return false;

	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel->GetModel();
	if (UnLive2DModel == nullptr) return false;

	if (DrawableIndexList.Num() != 0)
	{
		bool bUpdataSections = false;
		for (const uint16& DrawableIndex : DrawableIndexList)
		{
			if (UnLive2DModel->GetDrawableDynamicFlagRenderOrderDidChange(DrawableIndex) || UnLive2DModel->GetDrawableDynamicFlagVisibilityDidChange(DrawableIndex) /*|| UnLive2DModel->GetDrawableDynamicFlagOpacityDidChange(DrawableIndex)*/)
			{
				bUpdataSections = true;
				break;
			}
		}
		if (!bUpdataSections) return false;
	}
	const Csm::csmInt32 DrawableCount = UnLive2DModel->GetDrawableCount();

	const Csm::csmInt32* RenderOrder = UnLive2DModel->GetDrawableRenderOrders();

	OutSortedDrawableIndexList.SetNum(DrawableCount);
	DrawableIndexList.SetNum(DrawableCount);

	for (csmInt32 i = 0; i < DrawableCount; i++)
	{
		const csmInt32 Order = RenderOrder[i];
		OutSortedDrawableIndexList[Order] = i;
		DrawableIndexList[Order] = i;
	}
	return true;
}

bool IUnLive2DRenderBase::CreateClippingManager()
{
	check(UnLive2DRawModel.IsValid());
	UnLive2DClippingManager.Reset();
	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel->GetModel();
	const bool IsUsingMask = UnLive2DModel->IsUsingMasking();
	if (IsUsingMask)
	{
		UnLive2DClippingManager = MakeShared<CubismClippingManager_UE>();

		UnLive2DClippingManager->Initialize(
			UnLive2DModel,
			UnLive2DModel->GetDrawableCount(),
			UnLive2DModel->GetDrawableMasks(),
			UnLive2DModel->GetDrawableMaskCounts()
		);
		return true;
	}
	return false;
}

uint8 IUnLive2DRenderBase::GetUnLive2DShaderFlagsByDrawableIndex(const uint16& InDrawableIndex, uint8& MaskID) const
{
	check(UnLive2DRawModel.IsValid());
	Csm::CubismModel* InUnLive2DModel = UnLive2DRawModel->GetModel();
	uint8 Flags = EUnLive2DShaderFlags::BlendMode_Nor;
	MaskID = 0;
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
	if (UnLive2DClippingManager.IsValid())
	{
		if (auto ClipContext = UnLive2DClippingManager->GetClipContextInDrawableIndex(InDrawableIndex))
		{
			MaskID = ClipContext->ClippingManager_UID;
			Flags |= EUnLive2DShaderFlags::Mesk;
			if (InUnLive2DModel->GetDrawableInvertedMask(InDrawableIndex))
			{
				Flags |= EUnLive2DShaderFlags::InvertedMesk;
			}
		}
	}

	return Flags;
}

bool IUnLive2DRenderBase::GetDrawableDynamicIsVisible(const uint16& InDrawableIndex) const
{
	check(UnLive2DRawModel.IsValid());
	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel->GetModel();
	// <Drawable如果不是显示状态，则通过处理
	if (!UnLive2DModel->GetDrawableDynamicFlagIsVisible(InDrawableIndex)) return false;

	if (0 == UnLive2DModel->GetDrawableVertexIndexCount(InDrawableIndex)) return false;

	csmFloat32 Opacity = UnLive2DModel->GetDrawableOpacity(InDrawableIndex); // 获取不透明度
	return Opacity > 0.f;
}

bool IUnLive2DRenderBase::IsCombinedbBatchDidChange(const TArray<int32>& InDrawableCounts) const
{
	check(UnLive2DRawModel.IsValid());
	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel->GetModel();

	for (const int32& DrawableIndex : InDrawableCounts)
	{
		if (UnLive2DModel->GetDrawableDynamicFlagVertexPositionsDidChange(DrawableIndex)) return true;
	}

	return false;
}

void IUnLive2DRenderBase::ClearRenderBaseData()
{
	DrawableIndexList.Empty();
}

bool IUnLive2DRTRender::UpdateVertexBySectionData(FUnLive2DRTSectionData& InSectionData) const
{
	check(UnLive2DRawModel.IsValid());
	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel->GetModel();

	bool bCreateIndexBuffers = InSectionData.CacheIndexBuffersRHI.IsValid();
	if (bCreateIndexBuffers && !IsCombinedbBatchDidChange(InSectionData.DrawableCounts)) return false;
	InSectionData.bMarkDirty = true;
	InSectionData.Vertexs.Empty();
	for (const int32& DrawableIndex : InSectionData.DrawableCounts)
	{
		const csmInt32 NumVertext = UnLive2DModel->GetDrawableVertexCount(DrawableIndex); // 获得Drawable顶点的个数
		if (!bCreateIndexBuffers)
		{
			const csmInt32 VertexIndexCount = UnLive2DModel->GetDrawableVertexIndexCount(DrawableIndex); // 获得Drawable的顶点索引个数
			const csmUint16* IndicesArray = UnLive2DModel->GetDrawableVertexIndices(DrawableIndex); //顶点索引

			for (int32 Index = 0; Index < VertexIndexCount; ++Index)
			{
				InSectionData.Indices.Add(InSectionData.CacheVerticesIndexCount + IndicesArray[Index]);
			}
			InSectionData.CacheIndexCount += VertexIndexCount;
			InSectionData.CacheVerticesIndexCount += NumVertext;
		}
		csmFloat32 Opacity = UnLive2DModel->GetDrawableOpacity(DrawableIndex); // 获取不透明度

		const csmFloat32* VertexArray = UnLive2DModel->GetDrawableVertices(DrawableIndex); // 顶点组
		const  Live2D::Cubism::Core::csmVector2* UVArray = UnLive2DModel->GetDrawableVertexUvs(DrawableIndex); // 获取UV组

		FLinearColor BaseColor = FLinearColor(1.f, 1.f, 1.f, Opacity);
		for (int32 VertexIndex = 0; VertexIndex < NumVertext; ++VertexIndex)
		{
			FUnLive2DRenderTargetVertex& Vert = InSectionData.Vertexs.AddDefaulted_GetRef();
			Vert.Position = FULVector2f(VertexArray[VertexIndex * 2], VertexArray[VertexIndex * 2 + 1]);
			Vert.UV = FULVector2f(UVArray[VertexIndex].X, UVArray[VertexIndex].Y);
			Vert.Color = BaseColor;

		}
	}
	return true;
}

void IUnLive2DRTRender::GetVertexBySectionData(FRHICommandListImmediate& RHICmdList, FUnLive2DRTSectionData& SectionData)
{
	if (!SectionData.bMarkDirty) return;
	SectionData.bMarkDirty = false;
	// UpDataBuffers
	{
		if (!SectionData.CacheIndexBuffersRHI.IsValid())
		{
			const uint32 IndexSize = SectionData.CacheIndexCount * sizeof(uint16);
			FRHIResourceCreateInfo CreateInfo(TEXT("UnLive2DCacheIndexBuffer"));
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
			SectionData.CacheIndexBuffersRHI = RHICmdList.CreateIndexBuffer(sizeof(uint16), IndexSize, BUF_Static, CreateInfo);
#else
			SectionData.CacheIndexBuffersRHI = RHICreateIndexBuffer(sizeof(uint16), IndexSize, BUF_Static, CreateInfo);
#endif
#if ENGINE_MAJOR_VERSION >= 5
			void* IndexBufferData = RHICmdList.LockBuffer(SectionData.CacheIndexBuffersRHI, 0, IndexSize, RLM_WriteOnly);
			FMemory::Memmove(IndexBufferData, SectionData.Indices.GetData(), IndexSize);
			RHICmdList.UnlockBuffer(SectionData.CacheIndexBuffersRHI);
#else
			void* IndexBufferData = RHILockIndexBuffer(SectionData.CacheIndexBuffersRHI, 0, IndexSize, RLM_WriteOnly);
			FMemory::Memmove(IndexBufferData, SectionData.Indices.GetData(), IndexSize);
			RHIUnlockIndexBuffer(SectionData.CacheIndexBuffersRHI);
#endif
			SectionData.Indices.Empty();
		}
		const uint32 VertexSize = SectionData.CacheVerticesIndexCount * sizeof(FUnLive2DRenderTargetVertex);
		if (!SectionData.CacheVertexBufferRHI.IsValid())
		{
			FRHIResourceCreateInfo CreateInfo(TEXT("UnLive2DCacheVertexBufferRHI"));
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
			SectionData.CacheVertexBufferRHI = RHICmdList.CreateVertexBuffer(VertexSize, BUF_Dynamic, CreateInfo);
#else
			SectionData.CacheVertexBufferRHI = RHICreateVertexBuffer(VertexSize, BUF_Dynamic, CreateInfo);
#endif
		}
#if ENGINE_MAJOR_VERSION >= 5
		void* VertexBufferData = RHICmdList.LockBuffer(SectionData.CacheVertexBufferRHI, 0, VertexSize, RLM_WriteOnly);
		FMemory::Memmove(VertexBufferData, SectionData.Vertexs.GetData(), VertexSize);
		RHICmdList.UnlockBuffer(SectionData.CacheVertexBufferRHI);
#else
		void* VertexBufferData = RHILockVertexBuffer(SectionData.CacheVertexBufferRHI, 0, VertexSize, RLM_WriteOnly);
		FMemory::Memmove(VertexBufferData, SectionData.Vertexs.GetData(), VertexSize);
		RHIUnlockVertexBuffer(SectionData.CacheVertexBufferRHI);
#endif
	}
}

bool IUnLive2DRTRender::UpdataRTSections(bool bCombinedbBatch)
{
	check(UnLive2DRawModel.IsValid());
	TArray<uint16> SortedDrawableIndexList;
	if (!UpDataDrawableIndexList(SortedDrawableIndexList)) return UpDataSeparate_GameThread();

	UnLive2DSectionDataArr.Empty();
	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel->GetModel();
	for (const uint16& DrawableIndex : SortedDrawableIndexList)
	{
		if (!GetDrawableDynamicIsVisible(DrawableIndex)) continue;

		const csmInt32 TextureIndex = UnLive2DModel->GetDrawableTextureIndices(DrawableIndex);

		uint8 MaskID;
		uint8 Flags = GetUnLive2DShaderFlagsByDrawableIndex(DrawableIndex, MaskID);
		FUnLive2DRTSectionData* MeshSectionData = nullptr;
		if (!bCombinedbBatch || UnLive2DSectionDataArr.Num() == 0 || !UnLive2DSectionDataArr.Top().Equals(Flags, TextureIndex, MaskID))
		{
			MeshSectionData = &UnLive2DSectionDataArr.AddDefaulted_GetRef();
			MeshSectionData->Flags = Flags;
			MeshSectionData->TextureIndex = TextureIndex;
			MeshSectionData->CacheIndexCount = 0;
			MeshSectionData->MaskUID = MaskID;
			MeshSectionData->bMarkDirty = false;
			MeshSectionData->CacheVerticesIndexCount = 0;
		}
		else
			MeshSectionData = &UnLive2DSectionDataArr.Top();

		MeshSectionData->DrawableCounts.Add(DrawableIndex);
	}
	UpDataSeparate_GameThread();
	return true;
}

bool IUnLive2DRTRender::UpDataSeparate_GameThread()
{
	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel->GetModel();
	if (UnLive2DModel == nullptr) return false;
	bool bUpDate = false;
	for (FUnLive2DRTSectionData& SectionData : UnLive2DSectionDataArr)
	{
		if (UpdateVertexBySectionData(SectionData))
		{
			bUpDate = true;
		}
	}

	return bUpDate;
}

void IUnLive2DRTRender::DrawSeparateToRenderTarget_RenderThread(FRHICommandListImmediate& RHICmdList, FTextureRenderTargetResource* OutTextureRenderTargetResource, ERHIFeatureLevel::Type FeatureLevel, FTextureRHIRef InMaskBuffer)
{
	if (OutTextureRenderTargetResource == nullptr || !UnLive2DRawModel.IsValid()) return;

#if ENGINE_MAJOR_VERSION >=5 && ENGINE_MINOR_VERSION > 1
	const FTextureRHIRef RenderTargetTexture = OutTextureRenderTargetResource->GetRenderTargetTexture();
#else
	FRHITexture2D* RenderTargetTexture = OutTextureRenderTargetResource->GetRenderTargetTexture();
#endif
	// DrawClear
	{
#if ENGINE_MAJOR_VERSION < 5
		RHICmdList.TransitionResource(ERHIAccess::WritableMask, RenderTargetTexture);
#else
		RHICmdList.Transition(FRHITransitionInfo(RenderTargetTexture, ERHIAccess::WritableMask));
#endif
		FRHIRenderPassInfo RPInfo(RenderTargetTexture, ERenderTargetActions::Clear_Store);
		RHICmdList.BeginRenderPass(RPInfo, TEXT("DrawClear"));
		RHICmdList.EndRenderPass();
	}

	// Update viewport.
	RHICmdList.SetViewport(0.f, 0.f, 0.f, OutTextureRenderTargetResource->GetSizeX(), OutTextureRenderTargetResource->GetSizeY(), 1.f);
	
	for (FUnLive2DRTSectionData& SectionData : UnLive2DSectionDataArr)
	{
		bool bMask = SectionData.Flags & EUnLive2DShaderFlags::Mesk;
		GetVertexBySectionData(RHICmdList, SectionData);
		// DrawSeparate
		{
#if ENGINE_MAJOR_VERSION < 5
			RHICmdList.TransitionResource(ERHIAccess::EWritable, RenderTargetTexture);
#else
			RHICmdList.Transition(FRHITransitionInfo(RenderTargetTexture, ERHIAccess::WritableMask));
#endif
			FRHIRenderPassInfo RPInfo(RenderTargetTexture, ERenderTargetActions::Load_Store);
			RHICmdList.BeginRenderPass(RPInfo, TEXT("DrawSeparate"));
			{

				FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);

				// Set the graphic pipeline state.
				FGraphicsPipelineStateInitializer GraphicsPSOInit;
				RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
				GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
				GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
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
				GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GUnLive2DTargetVertexDeclaration.VertexDeclarationRHI;
				TShaderMapRef<FUnLive2DTargetShaderVS_Base> VertexShader(GlobalShaderMap);
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
				if (bMask)
				{
					FUnLiveMatrix MartixForMask;
					FUnLiveVector4 MaskPos;
					uint16 DrawableIndex = SectionData.DrawableCounts.Top();
					UnLive2DClippingManager->GetFillMaskMartixForMask(DrawableIndex, MartixForMask, MaskPos);
					const CubismRenderer::CubismTextureColor* ChanelFlagColor = UnLive2DClippingManager->GetChannelFlagAsColorByDrawableIndex(DrawableIndex);
					FLinearColor ClipColor = ChanelFlagColor == nullptr ? FLinearColor::White : FLinearColor(ChanelFlagColor->R, ChanelFlagColor->G, ChanelFlagColor->B, ChanelFlagColor->A);

					if (SectionData.Flags & EUnLive2DShaderFlags::InvertedMesk)
					{
						TShaderMapRef<FUnLive2DTargetShaderPS_Seq_InvertMask>PixelShader(GlobalShaderMap);
						GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
#if ENGINE_MAJOR_VERSION >= 5
						SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);
#else
						SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
#endif
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
						SetShaderParametersLegacyPS(RHICmdList, PixelShader, TextureResource, InMaskBuffer->GetTexture2D(), MartixForMask, ClipColor);
#else
						PixelShader->SetParameters(RHICmdList, PixelShader.GetPixelShader(), TextureResource, InMaskBuffer->GetTexture2D(), MartixForMask, ClipColor);
#endif
					}
					else
					{
						TShaderMapRef<FUnLive2DTargetShaderPS_Seq_Mask>PixelShader(GlobalShaderMap);
						GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
#if ENGINE_MAJOR_VERSION >= 5
						SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);
#else
						SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
#endif
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
						SetShaderParametersLegacyPS(RHICmdList, PixelShader, TextureResource, InMaskBuffer->GetTexture2D(), MartixForMask, ClipColor);
#else
						PixelShader->SetParameters(RHICmdList, PixelShader.GetPixelShader(), TextureResource, InMaskBuffer->GetTexture2D(), MartixForMask, ClipColor);
#endif
					}
				}
				else
				{
					TShaderMapRef<FUnLive2DTargetShaderPS_Nor>PixelShader(GlobalShaderMap);
					GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
#if ENGINE_MAJOR_VERSION >= 5
					SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);
#else
					SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
#endif
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
					SetShaderParametersLegacyPS(RHICmdList, PixelShader, TextureResource);
#else
					PixelShader->SetParameters(RHICmdList, PixelShader.GetPixelShader(), TextureResource);
#endif
				}

				RHICmdList.SetStreamSource(0, SectionData.CacheVertexBufferRHI, 0);
				RHICmdList.DrawIndexedPrimitive(
					SectionData.CacheIndexBuffersRHI,
					/*BaseVertexIndex=*/ 0,
					/*MinIndex=*/ 0,
					/*NumVertices=*/ SectionData.CacheVerticesIndexCount,
					/*StartIndex=*/ 0,
					/*NumPrimitives=*/ SectionData.CacheIndexCount / 3,
					/*NumInstances=*/ 1

				);

			}
			RHICmdList.EndRenderPass();
		}
	}
}

void IUnLive2DRTRender::ClearSectionData()
{
	UnLive2DSectionDataArr.Empty();
}

IUnLive2DRTRender::FUnLive2DRTSectionData::~FUnLive2DRTSectionData()
{
	CacheIndexBuffersRHI.SafeRelease();
	CacheVertexBufferRHI.SafeRelease();
}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
void IUnLive2DRTRender::FUnLive2DTargetVertexDeclaration::InitRHI(FRHICommandListBase& RHICmdList)
#else
void IUnLive2DRTRender::FUnLive2DTargetVertexDeclaration::InitRHI()
#endif
{
	FVertexDeclarationElementList Elements;
	uint16 Stride = sizeof(FUnLive2DRenderTargetVertex);
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUnLive2DRenderTargetVertex, Position), VET_Float2, 0, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUnLive2DRenderTargetVertex, UV), VET_Float2, 1, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUnLive2DRenderTargetVertex, Color), VET_Float4, 2, Stride));

	VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
}

void IUnLive2DRTRender::FUnLive2DTargetVertexDeclaration::ReleaseRHI()
{
	VertexDeclarationRHI.SafeRelease();
}
