#include "UnLive2DSepRenderer.h"
#include "UnLive2D.h"
#include "CubismConfig.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ImageUtils.h"
#include "UnLive2DRendererComponent.h"
#include "FWPort/UnLive2DRawModel.h"
#include "FWPort/UnLive2DModelRender.h"
#include "Engine/TextureRenderTarget2D.h"
#include "UnLive2DAsset.h"
#include "ModelRenders.h"
#include "PipelineStateCache.h"

FName MaskTextureParameterName = "UnLive2DMask";
FName MaskParmeterIsMeskName = "IsMask";
FName TintColorAndOpacityName = "TintColorAndOpacity";

FMatrix ConvertCubismMatrix(Csm::CubismMatrix44& InCubismMartix)
{
	FMatrix Matrix;

	Matrix.M[0][0] = InCubismMartix.GetArray()[0];
	Matrix.M[0][1] = InCubismMartix.GetArray()[1];
	Matrix.M[0][2] = InCubismMartix.GetArray()[2];
	Matrix.M[0][3] = InCubismMartix.GetArray()[3];

	Matrix.M[1][0] = InCubismMartix.GetArray()[4];
	Matrix.M[1][1] = InCubismMartix.GetArray()[5];
	Matrix.M[1][2] = InCubismMartix.GetArray()[6];
	Matrix.M[1][3] = InCubismMartix.GetArray()[7];

	Matrix.M[2][0] = InCubismMartix.GetArray()[8];
	Matrix.M[2][1] = InCubismMartix.GetArray()[9];
	Matrix.M[2][2] = InCubismMartix.GetArray()[10];
	Matrix.M[2][3] = InCubismMartix.GetArray()[11];

	Matrix.M[3][0] = InCubismMartix.GetArray()[12];
	Matrix.M[3][1] = InCubismMartix.GetArray()[13];
	Matrix.M[3][2] = InCubismMartix.GetArray()[14];
	Matrix.M[3][3] = InCubismMartix.GetArray()[15];

	return Matrix;
}

class FUnLive2DMaskShader : public FGlobalShader
{
	DECLARE_INLINE_TYPE_LAYOUT(FUnLive2DMaskShader, NonVirtual);;

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}

	FUnLive2DMaskShader() {}

	FUnLive2DMaskShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		TestFloat.Bind(Initializer.ParameterMap, TEXT("TestFloat"));
		ProjectMatrix.Bind(Initializer.ParameterMap, TEXT("ProjectMatrix"));
		BaseColor.Bind(Initializer.ParameterMap, TEXT("BaseColor"));
		ChannelFlag.Bind(Initializer.ParameterMap, TEXT("ChannelFlag"));

		MainTexture.Bind(Initializer.ParameterMap, TEXT("MainTexture"));
		MainTextureSampler.Bind(Initializer.ParameterMap, TEXT("MainTextureSampler"));
	}

	template<typename TShaderRHIParamRef>
	void SetParameters(
		FRHICommandListImmediate& RHICmdList,
		const TShaderRHIParamRef ShaderRHI,
		const FMatrix& InProjectMatrix,
		const FVector4& InBaseColor,
		const FVector4& InChannelFlag,
		FTextureRHIRef ShaderResourceTexture
	)
	{
		SetShaderValue(RHICmdList, ShaderRHI, TestFloat, 1.0f);
		SetShaderValue(RHICmdList, ShaderRHI, ProjectMatrix, InProjectMatrix);
		SetShaderValue(RHICmdList, ShaderRHI, BaseColor, InBaseColor);
		SetShaderValue(RHICmdList, ShaderRHI, ChannelFlag, InChannelFlag);

		SetTextureParameter(RHICmdList, ShaderRHI, MainTexture, ShaderResourceTexture);
		SetSamplerParameter(RHICmdList, ShaderRHI, MainTextureSampler, TStaticSamplerState<SF_Trilinear, AM_Wrap, AM_Wrap, AM_Wrap>::GetRHI());
	}

private:
	LAYOUT_FIELD(FShaderParameter, TestFloat);
	LAYOUT_FIELD(FShaderParameter, ProjectMatrix);
	LAYOUT_FIELD(FShaderParameter, BaseColor);
	LAYOUT_FIELD(FShaderParameter, ChannelFlag);

	LAYOUT_FIELD(FShaderResourceParameter, MainTexture);
	LAYOUT_FIELD(FShaderResourceParameter, MainTextureSampler);
};

class FUnLive2DMaskVS : public FUnLive2DMaskShader
{
	DECLARE_SHADER_TYPE(FUnLive2DMaskVS, Global);

public:

	/** Default constructor. */
	FUnLive2DMaskVS() {}

	/** Initialization constructor. */
	FUnLive2DMaskVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FUnLive2DMaskShader(Initializer)
	{
	}

private:
};

class FUnLive2DMaskPS : public FUnLive2DMaskShader
{
	DECLARE_SHADER_TYPE(FUnLive2DMaskPS, Global);

public:

	/** Default constructor. */
	FUnLive2DMaskPS() {}

	/** Initialization constructor. */
	FUnLive2DMaskPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FUnLive2DMaskShader(Initializer)
	{

	}
};

IMPLEMENT_SHADER_TYPE(, FUnLive2DMaskVS, TEXT("/Plugin/UnLive2DAsset/Private/CubismMask.usf"), TEXT("MainVS"), SF_Vertex)
IMPLEMENT_SHADER_TYPE(, FUnLive2DMaskPS, TEXT("/Plugin/UnLive2DAsset/Private/CubismMask.usf"), TEXT("MainPS"), SF_Pixel)

FUnLive2DRenderState::FUnLive2DRenderState(UUnLive2DRendererComponent* InComp)
	: UnLive2DClippingManager(nullptr)
	, MaskBufferRenderTarget(nullptr)
	, bNoLowPreciseMask(false)
	, OwnerCompWeak(InComp)
{
	if (InComp == nullptr) return;

	
}

FUnLive2DRenderState::~FUnLive2DRenderState()
{
	MaskRenderBuffers.Reset();
	UnLive2DClippingManager.Reset();
	UnLive2DToNormalBlendMaterial.Empty();
	UnLive2DToAdditiveBlendMaterial.Empty();
	UnLive2DToMultiplyBlendMaterial.Empty();

	if (MaskBufferRenderTarget != nullptr)
	{
		MaskBufferRenderTarget->RemoveFromRoot();
	}
}

void FUnLive2DRenderState::InitRender(TWeakObjectPtr<UUnLive2D> InNewUnLive2D)
{
	if (!InNewUnLive2D.IsValid()) return;

	if (!OwnerCompWeak.IsValid()) return;

	UUnLive2D* SourceUnLive2D = InNewUnLive2D.Get();

	if (!SourceUnLive2D->GetUnLive2DRawModel().IsValid()) return;

	LoadTextures();

	Csm::CubismModel* UnLive2DModel = SourceUnLive2D->GetUnLive2DRawModel().Pin()->GetModel();

	InitRenderBuffers();

	UnLive2DClippingManager.Reset();
	UnLive2DClippingManager = MakeShared<CubismClippingManager_UE>();

	UnLive2DClippingManager->Initialize(
		UnLive2DModel,
		UnLive2DModel->GetDrawableCount(),
		UnLive2DModel->GetDrawableMasks(),
		UnLive2DModel->GetDrawableMaskCounts()
	);

	const csmInt32 BufferHeight = UnLive2DClippingManager->GetClippingMaskBufferSize();

	MaskBufferRenderTarget = NewObject<UTextureRenderTarget2D>(OwnerCompWeak.Get());
	MaskBufferRenderTarget->ClearColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	MaskBufferRenderTarget->InitCustomFormat(BufferHeight, BufferHeight, EPixelFormat::PF_B8G8R8A8, false);
}

bool FUnLive2DRenderState::GetUseHighPreciseMask() const
{

    if (UUnLive2D* UnLive2D = OwnerCompWeak->SourceUnLive2D)
    {
        if (!UnLive2D->ModelConfigData.bTryLowPreciseMask)
        {
             return true;
        }
    }

    /** 
     * Low precise is not possible
     * @Note See SetupClippingContext for detail
     */
    if (bNoLowPreciseMask)
    {
        return true;
    }

    return false;
}

void FUnLive2DRenderState::LoadTextures()
{
	if (!OwnerCompWeak.IsValid()) return;

	UUnLive2D* SourceUnLive2D = OwnerCompWeak->SourceUnLive2D;

	if (SourceUnLive2D == nullptr) return;

	TWeakPtr<FUnLive2DRawModel> ModelWeakPtr = SourceUnLive2D->GetUnLive2DRawModel();

	if (!ModelWeakPtr.IsValid()) return;

	TWeakPtr<Csm::ICubismModelSetting> ModelSettingWeakPtr = ModelWeakPtr.Pin()->GetModelSetting();

	if (!ModelSettingWeakPtr.IsValid()) return;
	TSharedPtr<Csm::ICubismModelSetting> ModelSetting = ModelSettingWeakPtr.Pin();
	csmInt32 TextureNum = ModelSetting->GetTextureCount();

	// 清理老的图片内存
	UnLoadTextures();
	RandererStatesTextures.SetNum(TextureNum);

	// 加载新的图片
	const FUnLive2DLoadData* LoadData = SourceUnLive2D->GetUnLive2DLoadData();
	if (LoadData == nullptr) return;

	for (int32 i = 0; i < LoadData->Live2DTexture2DData.Num(); i++)
	{
		UTexture2D* LoadedImage = FImageUtils::ImportBufferAsTexture2D(LoadData->Live2DTexture2DData[i].ByteData);
		if (IsValid(LoadedImage))
		{
			RandererStatesTextures[i] = LoadedImage;
		}
		LoadedImage->AddToRoot();
	}

	UE_LOG(LogUnLive2D, Log, TEXT("加载了 %d 数量的图片"), ModelSetting->GetTextureCount());
}

void FUnLive2DRenderState::UnLoadTextures()
{
	for (TWeakObjectPtr<UTexture2D>& Item : RandererStatesTextures)
	{
		if (!Item.IsValid()) continue;

		Item->RemoveFromRoot();
	}

	RandererStatesTextures.Empty();
}

class UTexture2D* FUnLive2DRenderState::GetRandererStatesTexturesTextureIndex(Csm::CubismModel* Live2DModel, const Csm::csmInt32& DrawableIndex) const
{

	const csmInt32 TextureIndex = Live2DModel->GetDrawableTextureIndices(DrawableIndex);
	if (!RandererStatesTextures.IsValidIndex(TextureIndex)) return nullptr;

	TWeakObjectPtr<UTexture2D> TextureWeakPtr = RandererStatesTextures[TextureIndex];

	if (!TextureWeakPtr.IsValid()) return nullptr;

	return TextureWeakPtr.Get();
}

UMaterialInstanceDynamic* FUnLive2DRenderState::GetMaterialInstanceDynamicToIndex(Csm::CubismModel* Live2DModel, const Csm::csmInt32 DrawableIndex, bool bIsMesk)
{
	if (!OwnerCompWeak.IsValid()) return nullptr;

	UUnLive2D* UnLive2D = OwnerCompWeak->SourceUnLive2D;
	if (UnLive2D == nullptr) return nullptr;

	const csmInt32 TextureIndex = Live2DModel->GetDrawableTextureIndices(DrawableIndex);
	Rendering::CubismRenderer::CubismBlendMode BlendMode = Live2DModel->GetDrawableBlendMode(DrawableIndex);
	int32 IsMeskValue = bIsMesk ? 1 : 0;
	int32 BlendModeIndex = (BlendMode + 1) * 1000; //根据绘制类型和图片ID来判断是否在缓存中储存了该动态材质
	//int32 MapIndex = BlendModeIndex + (TextureIndex  * 10) + bIsMesk ? 1 : 0;
	int32 MapIndex = BlendModeIndex + (TextureIndex * 10) + IsMeskValue;

	auto SetMaterialInstanceDynamicParameter = [=](UMaterialInstanceDynamic* DynamicMat)
	{
		UTexture2D* Texture = GetRandererStatesTexturesTextureIndex(Live2DModel, DrawableIndex);
		check(Texture && "Texture Is Null");
		DynamicMat->SetTextureParameterValue(UnLive2D->TextureParameterName, Texture);
		DynamicMat->SetTextureParameterValue(MaskTextureParameterName, MaskBufferRenderTarget);
		DynamicMat->SetScalarParameterValue(MaskParmeterIsMeskName, IsMeskValue);
		DynamicMat->SetVectorParameterValue(TintColorAndOpacityName, UnLive2D->TintColorAndOpacity);
	};

	UMaterialInstanceDynamic* Material = nullptr;
	switch (BlendMode)
	{
	case Rendering::CubismRenderer::CubismBlendMode_Normal:
	{
		UMaterialInstanceDynamic*& FindMaterial = UnLive2DToNormalBlendMaterial.FindOrAdd(MapIndex);
		if (FindMaterial == nullptr)
		{
			FindMaterial = UMaterialInstanceDynamic::Create(UnLive2D->UnLive2DNormalMaterial, OwnerCompWeak.Get());
			SetMaterialInstanceDynamicParameter(FindMaterial);
		}
		Material = FindMaterial;
	}
	break;
	case Rendering::CubismRenderer::CubismBlendMode_Additive:
	{
		UMaterialInstanceDynamic*& FindMaterial = UnLive2DToAdditiveBlendMaterial.FindOrAdd(MapIndex);
		if (FindMaterial == nullptr)
		{
			FindMaterial = UMaterialInstanceDynamic::Create(UnLive2D->UnLive2DAdditiveMaterial, OwnerCompWeak.Get());
			SetMaterialInstanceDynamicParameter(FindMaterial);
		}
		Material = FindMaterial;
	}
	break;
	case Rendering::CubismRenderer::CubismBlendMode_Multiplicative:
	{
		UMaterialInstanceDynamic*& FindMaterial = UnLive2DToMultiplyBlendMaterial.FindOrAdd(MapIndex);
		if (FindMaterial == nullptr)
		{
			FindMaterial = UMaterialInstanceDynamic::Create(UnLive2D->UnLive2DMultiplyMaterial, OwnerCompWeak.Get());
			SetMaterialInstanceDynamicParameter(FindMaterial);
		}
		Material = FindMaterial;
	}
	break;
	}

	return Material;
}

CubismClippingContext* FUnLive2DRenderState::GetClipContextInDrawableIndex(const Csm::csmUint32 DrawableIndex) const
{
	if (!UnLive2DClippingManager.IsValid()) return nullptr;

	csmVector<CubismClippingContext*>* ClippingContextArr = UnLive2DClippingManager->GetClippingContextListForDraw();
	if (ClippingContextArr != nullptr && DrawableIndex >= 0 && ClippingContextArr->GetSize() > DrawableIndex)
	{
		return (*ClippingContextArr)[DrawableIndex];
	}

	return nullptr;
}

void FUnLive2DRenderState::InitRenderBuffers()
{
	check(IsInGameThread());

	Csm::CubismModel* UnLive2DModel = OwnerCompWeak->SourceUnLive2D->GetUnLive2DRawModel().Pin()->GetModel();

	Csm::csmInt32 DrawableCount = UnLive2DModel->GetDrawableCount();

	MaskRenderBuffers = MakeShared<FUnLive2DRenderBuffers>();

	ENQUEUE_RENDER_COMMAND(UnLiveRenderInit)([=](FRHICommandListImmediate& RHICmdList)
	{
		check(IsInRenderingThread()); // 如果不是渲染线程请弄成渲染线程

		if (!MaskRenderBuffers.IsValid()) return;

		for (Csm::csmInt32 DrawIter = 0; DrawIter < DrawableCount; ++DrawIter)
		{
			/** Vertex buffer */
			{
				const Csm::csmInt32 VCount = UnLive2DModel->GetDrawableVertexCount(DrawIter);
				if (VCount == 0) continue;
				FRHIResourceCreateInfo CreateInfoVert;
				void* DrawableData = nullptr;
				FVertexBufferRHIRef ScratchVertexBufferRHI = RHICreateAndLockVertexBuffer(VCount * sizeof(FCubismVertex), BUF_Dynamic, CreateInfoVert, DrawableData);
				RHIUnlockVertexBuffer(ScratchVertexBufferRHI);

				MaskRenderBuffers->VertexBuffers.Add(DrawIter, ScratchVertexBufferRHI);
				MaskRenderBuffers->VertexCounts.Add(DrawIter, VCount);
			}
			
			{
				/** Index buffer*/
				const csmInt32 IndexCount = UnLive2DModel->GetDrawableVertexIndexCount(DrawIter);

				if (IndexCount == 0) continue;

				const csmUint16* IndexArray = const_cast<csmUint16*>(UnLive2DModel->GetDrawableVertexIndices(DrawIter));
				FRHIResourceCreateInfo CreateInfoIndice;
				FIndexBufferRHIRef IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), sizeof(uint16) * IndexCount, BUF_Static, CreateInfoIndice);
				void* VoidPtr = RHILockIndexBuffer(IndexBufferRHI, 0, sizeof(uint16) * IndexCount, RLM_WriteOnly);
				FMemory::Memcpy(VoidPtr, IndexArray, IndexCount * sizeof(uint16));
				RHIUnlockIndexBuffer(IndexBufferRHI);

				MaskRenderBuffers->IndexBuffers.Add(DrawIter, IndexBufferRHI);
			}
		}
	});
}

void FUnLive2DRenderState::UpdateRenderBuffers()
{
	check(IsInGameThread());

	Csm::CubismModel* UnLive2DModel = OwnerCompWeak->SourceUnLive2D->GetUnLive2DRawModel().Pin()->GetModel();

	if (OwnerCompWeak->GetWorld() == nullptr) return;

	ERHIFeatureLevel::Type FeatureLevel = OwnerCompWeak->GetWorld()->Scene->GetFeatureLevel();

	ENQUEUE_RENDER_COMMAND(UpdateRender)([=](FRHICommandListImmediate& RHICmdList)
	{
		check(IsInRenderingThread()); // 如果不是渲染线程请弄成渲染线程

		if (!UnLive2DModel->IsUsingMasking() || !UnLive2DClippingManager.IsValid()) return;

		UnLive2DClippingManager->SetupClippingContext(UnLive2DModel, this);

		if (this->GetUseHighPreciseMask()) return;

		UpdateMaskBufferRenderTarget(RHICmdList, UnLive2DModel, FeatureLevel);
		
	});
}

void UnLive2DFillMaskParameter(CubismClippingContext* clipContext, CubismClippingManager_UE* _clippingManager, FMatrix& ts_MartixForMask, FVector4& ts_BaseColor, FVector4& ts_ChanelFlag)
{
	// チャンネル
	const csmInt32 channelNo = clipContext->_layoutChannelNo;
	// チャンネルをRGBAに変換
	Csm::Rendering::CubismRenderer::CubismTextureColor* colorChannel = _clippingManager->GetChannelFlagAsColor(channelNo);

	csmRectF* rect = clipContext->_layoutBounds;

	ts_MartixForMask = ConvertCubismMatrix(clipContext->_matrixForMask);
	ts_BaseColor = FVector4(rect->X * 2.0f - 1.0f, rect->Y * 2.0f - 1.0f, rect->GetRight() * 2.0f - 1.0f, rect->GetBottom() * 2.0f - 1.0f);
	ts_ChanelFlag = FVector4(colorChannel->R, colorChannel->G, colorChannel->B, colorChannel->A);
}

void FUnLive2DRenderState::UpdateMaskBufferRenderTarget(FRHICommandListImmediate& RHICmdList, Csm::CubismModel* tp_Model, ERHIFeatureLevel::Type FeatureLevel)
{
	if (RHICmdList.IsStalled()) return;

	FRHITexture2D* RenderTargetTexture = MaskBufferRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();

	RHICmdList.TransitionResource(ERHIAccess::EWritable, RenderTargetTexture);

	FRHIRenderPassInfo RPInfo(RenderTargetTexture, ERenderTargetActions::Clear_Store, RenderTargetTexture);
	RHICmdList.BeginRenderPass(RPInfo, TEXT("DrawMask01"));
	{
		float MaskSize = UnLive2DClippingManager->_clippingMaskBufferSize;

		RHICmdList.SetViewport(0.f, 0.f, 0.f, MaskSize, MaskSize, 1.f);

		FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);
		TShaderMapRef< FUnLive2DMaskVS > VertexShader(GlobalShaderMap);
		TShaderMapRef< FUnLive2DMaskPS > PixelShader(GlobalShaderMap);

		// Set the graphic pipeline state.
		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_Zero, BF_InverseSourceColor, BO_Add, BF_Zero, BF_InverseSourceAlpha>::GetRHI();
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, true, true>::GetRHI();
		GraphicsPSOInit.PrimitiveType = PT_TriangleList;
		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GCubismVertexDeclaration.VertexDeclarationRHI;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();

		for (csmUint32 clipIndex = 0; clipIndex < UnLive2DClippingManager->_clippingContextListForMask.GetSize(); clipIndex++)
		{
			CubismClippingContext* clipContext = UnLive2DClippingManager->_clippingContextListForMask[clipIndex];

			const csmInt32 clipDrawCount = clipContext->_clippingIdCount;
			for (csmInt32 i = 0; i < clipDrawCount; i++)
			{
				const csmInt32 clipDrawIndex = clipContext->_clippingIdList[i];

				// 頂点情報が更新されておらず、信頼性がない場合は描画をパスする
				if (!tp_Model->GetDrawableDynamicFlagVertexPositionsDidChange(clipDrawIndex))
				{
					continue;
				}

				// TODO: push [IsCulling] to [RasterizerState] if needed
				const bool tb_IsCulling = (tp_Model->GetDrawableCulling(clipDrawIndex) != 0);

				TWeakObjectPtr<UTexture2D> tp_Texture = GetRandererStatesTexturesTextureIndex(tp_Model, clipDrawIndex);

				if (tp_Texture == nullptr)
				{
					UE_LOG(LogUnLive2D, Warning, TEXT("DrawSeparateToRenderTarget_RenderThread:[Mask] Texture  invalid"));
					continue;
				}


				const csmInt32 indexCount = tp_Model->GetDrawableVertexIndexCount(clipDrawIndex);
				if (0 == indexCount)
				{
					continue;
				}

				/*if (!MaskRenderBuffers->IndexBuffers.Contains(clipDrawIndex))
				{
					continue;
				}

				if (!MaskRenderBuffers->VertexBuffers.Contains(clipDrawIndex))
				{
					UE_LOG(LogUnLive2D, Error, TEXT("DrawSeparateToRenderTarget_RenderThread:[Mask] Vertext buffer not inited."));
					continue;
				}*/

				//////////////////////////////////////////////////////////////////////////
				csmFloat32 tf_Opacity = tp_Model->GetDrawableOpacity(clipDrawIndex);
				Rendering::CubismRenderer::CubismBlendMode ts_BlendMode = tp_Model->GetDrawableBlendMode(clipDrawIndex);
				csmBool tb_InvertMask = tp_Model->GetDrawableInvertedMask(clipDrawIndex);


				//////////////////////////////////////////////////////////////////////////
				/** Drawable draw */
				const csmInt32 td_NumVertext = tp_Model->GetDrawableVertexCount(clipDrawIndex);

				FIndexBufferRHIRef IndexBufferRHI = MaskRenderBuffers->IndexBuffers.FindRef(clipDrawIndex);
				FVertexBufferRHIRef ScratchVertexBufferRHI = MaskRenderBuffers->VertexBuffers.FindRef(clipDrawIndex);
				FTextureRHIRef tsr_TextureRHI = tp_Texture->Resource->TextureRHI;

				MaskFillVertexBuffer(tp_Model, clipDrawIndex, ScratchVertexBufferRHI, RHICmdList);

				////////////////////////////////////////////////////////////////////////////
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

				FMatrix ts_MartixForMask;
				FVector4 ts_BaseColor;
				FVector4 ts_ChanelFlag;

				UnLive2DFillMaskParameter(clipContext, UnLive2DClippingManager.Get(), ts_MartixForMask, ts_BaseColor, ts_ChanelFlag);

				VertexShader->SetParameters(RHICmdList, VertexShader.GetVertexShader(), ts_MartixForMask, ts_BaseColor, ts_ChanelFlag, tsr_TextureRHI);
				PixelShader->SetParameters(RHICmdList, PixelShader.GetPixelShader(), ts_MartixForMask, ts_BaseColor, ts_ChanelFlag, tsr_TextureRHI);

				////////////////////////////////////////////////////////////////////////////

				RHICmdList.SetStreamSource(0, ScratchVertexBufferRHI, 0);

				RHICmdList.DrawIndexedPrimitive(
					IndexBufferRHI,
					/*BaseVertexIndex=*/ 0,
					/*MinIndex=*/ 0,
					/*NumVertices=*/ td_NumVertext,
					/*StartIndex=*/ 0,
					/*NumPrimitives=*/ indexCount / 3,
					/*NumInstances=*/ 1
				);

			}
		}
	}
	RHICmdList.EndRenderPass();
}

void FUnLive2DRenderState::MaskFillVertexBuffer(Csm::CubismModel* tp_Model, const Csm::csmInt32 drawableIndex, FVertexBufferRHIRef ScratchVertexBufferRHI, FRHICommandListImmediate& RHICmdList)
{
	const csmInt32 td_NumVertext = tp_Model->GetDrawableVertexCount(drawableIndex);
	UE_LOG(LogUnLive2D, Verbose, TEXT("FillVertexBuffer: Vertext buffer info %d|%d >> (%u, %u)"), drawableIndex, td_NumVertext, ScratchVertexBufferRHI->GetSize(), ScratchVertexBufferRHI->GetUsage());

	if (MaskRenderBuffers->VertexCounts.Find(drawableIndex) == nullptr) return;

	//check(td_NumVertext == tp_States->VertexCount[drawableIndex]);
	{

		const csmFloat32* uvarray = reinterpret_cast<csmFloat32*>(const_cast<Live2D::Cubism::Core::csmVector2*>(tp_Model->GetDrawableVertexUvs(drawableIndex)));
		const csmFloat32* varray = const_cast<csmFloat32*>(tp_Model->GetDrawableVertices(drawableIndex));

		check(varray);
		check(uvarray);

		void* DrawableData = RHICmdList.LockVertexBuffer(ScratchVertexBufferRHI, 0, td_NumVertext * sizeof(FCubismVertex), RLM_WriteOnly);
		FCubismVertex* RESTRICT DestSamples = (FCubismVertex*)DrawableData;

		for (int32 td_VertexIndex = 0; td_VertexIndex < td_NumVertext; ++td_VertexIndex)
		{
			DestSamples[td_VertexIndex].Position.X = varray[td_VertexIndex * 2];
			DestSamples[td_VertexIndex].Position.Y = varray[td_VertexIndex * 2 + 1];
			DestSamples[td_VertexIndex].UV.X = uvarray[td_VertexIndex * 2];
			DestSamples[td_VertexIndex].UV.Y = uvarray[td_VertexIndex * 2 + 1];
		}

		RHICmdList.UnlockVertexBuffer(ScratchVertexBufferRHI);
	}
}

FMatrix FUnLive2DRenderState::GetUnLive2DPosToClipMartix(class CubismClippingContext* ClipContext, FVector4& ChanelFlag)
{
	if (ClipContext == nullptr) return FMatrix();

	const csmInt32 ChannelNo = ClipContext->_layoutChannelNo; // 通道

	// 将通道转换为RGBA
	Csm::Rendering::CubismRenderer::CubismTextureColor* ColorChannel = UnLive2DClippingManager->GetChannelFlagAsColor(ChannelNo);

	ChanelFlag = FVector4(ColorChannel->R, ColorChannel->G, ColorChannel->B, ColorChannel->A);

	return ConvertCubismMatrix(ClipContext->_matrixForDraw);
}

void FUnLive2DRenderState::SetDynamicMaterialTintColor(FLinearColor& NewColor)
{
	for (auto& Item : UnLive2DToNormalBlendMaterial)
	{
		Item.Value->SetVectorParameterValue(TintColorAndOpacityName, NewColor);
	}
	for (auto& Item : UnLive2DToAdditiveBlendMaterial)
	{
		Item.Value->SetVectorParameterValue(TintColorAndOpacityName, NewColor);
	}
	for (auto& Item : UnLive2DToMultiplyBlendMaterial)
	{
		Item.Value->SetVectorParameterValue(TintColorAndOpacityName, NewColor);
	}
}

