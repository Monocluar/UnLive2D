#include "UnLive2DSepRenderer.h"
#include "UnLive2D.h"
#include "CubismConfig.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ImageUtils.h"
#include "UnLive2DRendererComponent.h"
#include "FWPort/UnLive2DRawModel.h"
#include "FWPort/UnLive2DModelRender.h"
#include "Engine/TextureRenderTarget2D.h"
#include "UnLive2DAssetModule.h"
#include "ModelRenders.h"
#include "PipelineStateCache.h"
#include "GlobalShader.h"
#include "ShaderParameterUtils.h"
#include "RHI.h"
#include "Slate/SUnLive2DViewUI.h"
#include "UnLive2DViewRendererUI.h"


FName MaskTextureParameterName = "UnLive2DMask";
FName MaskParmeterIsMeskName = "IsMask";
FName MaskParmterIsInvertedMaskName = "IsInvertedMask";
FName TintColorAndOpacityName = "TintColorAndOpacity";

FUnLiveMatrix ConvertCubismMatrix(Csm::CubismMatrix44& InCubismMartix)
{
	FUnLiveMatrix Matrix;

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
		//return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		return true;
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
		const FUnLiveMatrix& InProjectMatrix,
		const FUnLiveVector4& InBaseColor,
		const FUnLiveVector4& InChannelFlag,
		FTextureRHIRef ShaderResourceTexture
	)
	{
		SetShaderValue(RHICmdList, ShaderRHI, TestFloat, 1.f);
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
	, bNoLowPreciseMask(false)
	, OwnerCompWeak(InComp)
{
}

FUnLive2DRenderState::FUnLive2DRenderState(UUnLive2DViewRendererUI* InViewUI)
	: UnLive2DClippingManager(nullptr)
	, bNoLowPreciseMask(false)
	, OwnerViewUIWeak(InViewUI)
{

}

FUnLive2DRenderState::~FUnLive2DRenderState()
{
	UnLive2DClippingManager.Reset();

	UnLoadTextures();

	if (!MaskBufferRenderTarget.IsValid() && MaskBufferRenderTarget.Get() != nullptr)
	{
		MaskBufferRenderTarget->RemoveFromRoot();
	}
	UnOldMaterial();
}

void FUnLive2DRenderState::InitRender(TWeakObjectPtr<UUnLive2D> InNewUnLive2D, TSharedPtr<FUnLive2DRawModel> InUnLive2DRawModel)
{
	if (!InNewUnLive2D.IsValid() || !InUnLive2DRawModel.IsValid()) return;

	if (!OwnerCompWeak.IsValid() && !OwnerViewUIWeak.IsValid()) return;

	UUnLive2D* SourceUnLive2D = InNewUnLive2D.Get();

	InitRender(SourceUnLive2D, InUnLive2DRawModel);
}


void FUnLive2DRenderState::InitRender(const UUnLive2D* InNewUnLive2D, TSharedPtr<FUnLive2DRawModel> InUnLive2DRawModel)
{
	if (InNewUnLive2D == nullptr) return;

	if (!OwnerCompWeak.IsValid() && !OwnerViewUIWeak.IsValid()) return;

	if (!InUnLive2DRawModel.IsValid()) return;

	if (UnLive2DClippingManager.IsValid())
	{
		UnLive2DClippingManager.Reset();
		UnOldMaterial();
	}

	LoadTextures(InUnLive2DRawModel);

	Csm::CubismModel* UnLive2DModel = InUnLive2DRawModel->GetModel();

	InitRenderBuffers(InUnLive2DRawModel);

	UnLive2DClippingManager = MakeShared<CubismClippingManager_UE>();

	UnLive2DClippingManager->Initialize(
		UnLive2DModel,
		UnLive2DModel->GetDrawableCount(),
		UnLive2DModel->GetDrawableMasks(),
		UnLive2DModel->GetDrawableMaskCounts()
	);

	const csmInt32 BufferHeight = UnLive2DClippingManager->GetClippingMaskBufferSize();

	if (!MaskBufferRenderTarget.IsValid())
	{
		//MaskBufferRenderTarget = NewObject<UTextureRenderTarget2D>(GetTransientPackage(), NAME_None, RF_Transient);
		MaskBufferRenderTarget = NewObject<UTextureRenderTarget2D>(GetTransientPackage());
		MaskBufferRenderTarget->ClearColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
		MaskBufferRenderTarget->InitCustomFormat(BufferHeight, BufferHeight, EPixelFormat::PF_B8G8R8A8, false);
		MaskBufferRenderTarget->AddToRoot();
	}

}

bool FUnLive2DRenderState::GetUseHighPreciseMask() const
{

    if (const UUnLive2D* UnLive2D = GetUnLive2D())
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

void FUnLive2DRenderState::LoadTextures(TWeakPtr<FUnLive2DRawModel> InUnLive2DRawModel)
{
	if (!OwnerCompWeak.IsValid() && !OwnerViewUIWeak.IsValid()) return;

	const UUnLive2D* SourceUnLive2D = GetUnLive2D();

	if (SourceUnLive2D == nullptr) return;

	if (!InUnLive2DRawModel.IsValid()) return;

	const TWeakPtr<Csm::ICubismModelSetting> ModelSettingWeakPtr = InUnLive2DRawModel.Pin()->GetModelSetting();

	if (!ModelSettingWeakPtr.IsValid()) return;
	TSharedPtr<Csm::ICubismModelSetting> ModelSetting = ModelSettingWeakPtr.Pin();
	csmInt32 TextureNum = ModelSetting->GetTextureCount();

	// 清理老的图片内存
	UnLoadTextures();
	RandererStatesTextures.SetNum(TextureNum);

	// 加载新的图片
	for (int32 i = 0; i < TextureNum; i++)
	{
		if (!SourceUnLive2D->TextureAssets.IsValidIndex(i)) break;

		UTexture2D* LoadedImage = SourceUnLive2D->TextureAssets[i].LoadSynchronous();
		if (IsValid(LoadedImage))
		{
			RandererStatesTextures[i] = LoadedImage;
		}
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

void FUnLive2DRenderState::UnOldMaterial()
{
	for (auto& Item : UnLive2DToNormalBlendMaterial)
	{
		if (Item.Value == nullptr) continue;

		Item.Value->RemoveFromRoot();
	}
	UnLive2DToNormalBlendMaterial.Empty();
	for (auto& Item : UnLive2DToMultiplyBlendMaterial)
	{
		if (Item.Value == nullptr) continue;

		Item.Value->RemoveFromRoot();
	}
	UnLive2DToMultiplyBlendMaterial.Empty();
	for (auto& Item : UnLive2DToAdditiveBlendMaterial)
	{
		if (Item.Value == nullptr) continue;

		Item.Value->RemoveFromRoot();
	}
	UnLive2DToAdditiveBlendMaterial.Empty();
}

class UTexture2D* FUnLive2DRenderState::GetRandererStatesTexturesTextureIndex(const Csm::CubismModel* Live2DModel, const Csm::csmInt32& DrawableIndex) const
{

	const csmInt32 TextureIndex = Live2DModel->GetDrawableTextureIndices(DrawableIndex);
	if (!RandererStatesTextures.IsValidIndex(TextureIndex)) return nullptr;

	TWeakObjectPtr<UTexture2D> TextureWeakPtr = RandererStatesTextures[TextureIndex];

	if (!TextureWeakPtr.IsValid()) return nullptr;

	return TextureWeakPtr.Get();
}

UMaterialInstanceDynamic* FUnLive2DRenderState::GetMaterialInstanceDynamicToIndex(const Csm::CubismModel* Live2DModel, const Csm::csmInt32 DrawableIndex, bool bIsMesk)
{
	if (!OwnerCompWeak.IsValid() && !OwnerViewUIWeak.IsValid()) return nullptr;

	const UUnLive2D* UnLive2D = GetUnLive2D();
	if (UnLive2D == nullptr) return nullptr;

	int32 InvertedMesk = 0;
	if (bIsMesk && Live2DModel->GetDrawableInvertedMask(DrawableIndex)) //为遮罩材质并且是否该点是否是反转遮罩
	{
		InvertedMesk = 1;
	}

	const csmInt32 TextureIndex = Live2DModel->GetDrawableTextureIndices(DrawableIndex);
	Rendering::CubismRenderer::CubismBlendMode BlendMode = Live2DModel->GetDrawableBlendMode(DrawableIndex);
	int32 IsMeskValue = bIsMesk ? 1 : 0;
	int32 BlendModeIndex = (BlendMode + 1) * 10000; //根据绘制类型和图片ID来判断是否在缓存中储存了该动态材质
	int32 MapIndex = BlendModeIndex + (TextureIndex * 100) + (IsMeskValue * 10) + InvertedMesk;

	auto SetMaterialInstanceDynamicParameter = [=](UMaterialInstanceDynamic* DynamicMat)
	{
		UTexture2D* Texture = GetRandererStatesTexturesTextureIndex(Live2DModel, DrawableIndex);
		//check(Texture && "Texture Is Null");
		if (Texture == nullptr)
		{
			return;
		}
		DynamicMat->SetTextureParameterValue(GetDMaterialTextureParameterName(), Texture);
		DynamicMat->SetTextureParameterValue(MaskTextureParameterName, MaskBufferRenderTarget.Get());
		DynamicMat->SetScalarParameterValue(MaskParmeterIsMeskName, IsMeskValue);
		DynamicMat->SetScalarParameterValue(MaskParmterIsInvertedMaskName, InvertedMesk);
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
			FindMaterial = GetUnLive2DMaterial((int32)BlendMode);
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
			FindMaterial = GetUnLive2DMaterial((int32)BlendMode);
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
			FindMaterial = GetUnLive2DMaterial((int32)BlendMode);
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

void FUnLive2DRenderState::InitRenderBuffers(TSharedPtr<FUnLive2DRawModel>& InUnLive2DRawModel)
{
	check(IsInGameThread());

	ENQUEUE_RENDER_COMMAND(UnLiveRenderInit)([ThisSharedPtr = AsShared(), InUnLive2DRawModel](FRHICommandListImmediate& RHICmdList)
	{
		check(IsInRenderingThread()); // 如果不是渲染线程请弄成渲染线程

		if (!InUnLive2DRawModel.IsValid()) return;

		if (!ThisSharedPtr->OwnerCompWeak.IsValid() && !ThisSharedPtr->OwnerViewUIWeak.IsValid()) return;

		Csm::CubismModel* UnLive2DModel = InUnLive2DRawModel->GetModel();

		if (UnLive2DModel == nullptr) return;

		Csm::csmInt32 DrawableCount = UnLive2DModel->GetDrawableCount();

		if (UnLive2DModel == nullptr) return;

		ThisSharedPtr->MaskRenderBuffers.Clear();

		for (Csm::csmInt32 DrawIter = 0; DrawIter < DrawableCount; ++DrawIter)
		{
			/** Vertex buffer */
			{
				const Csm::csmInt32 VCount = UnLive2DModel->GetDrawableVertexCount(DrawIter);
				if (VCount == 0) continue;
				FRHIResourceCreateInfo CreateInfoVert(TEXT(""));
				void* DrawableData = nullptr;
#if UE_VERSION_OLDER_THAN(5,0,0)
				FVertexUnLiveBufferRHIRef ScratchVertexBufferRHI = RHICreateAndLockVertexBuffer(VCount * sizeof(FCubismVertex), BUF_Static, CreateInfoVert, DrawableData);
				RHIUnlockVertexBuffer(ScratchVertexBufferRHI);
#else
				FBufferRHIRef ScratchVertexBufferRHI = FRHICommandListExecutor::GetImmediateCommandList().CreateAndLockVertexBuffer(VCount * sizeof(FCubismVertex),
					BUF_Static, CreateInfoVert, DrawableData);
				RHIUnlockBuffer(ScratchVertexBufferRHI);
#endif

				ThisSharedPtr->MaskRenderBuffers.VertexBuffers.Add(DrawIter, ScratchVertexBufferRHI);
				ThisSharedPtr->MaskRenderBuffers.VertexCounts.Add(DrawIter, VCount);
			}
			
			{
				/** Index buffer*/
				const csmInt32 IndexCount = UnLive2DModel->GetDrawableVertexIndexCount(DrawIter);

				if (IndexCount == 0) continue;

				const csmUint16* IndexArray = const_cast<csmUint16*>(UnLive2DModel->GetDrawableVertexIndices(DrawIter));
				FRHIResourceCreateInfo CreateInfoIndice(TEXT(""));
				FIndexUnLiveBufferRHIRef IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), sizeof(uint16) * IndexCount, BUF_Static, CreateInfoIndice);

#if UE_VERSION_OLDER_THAN(5,0,0)
				void* VoidPtr = RHILockIndexBuffer(IndexBufferRHI, 0, sizeof(uint16) * IndexCount, RLM_WriteOnly);
				FMemory::Memcpy(VoidPtr, IndexArray, IndexCount * sizeof(uint16));
				RHIUnlockIndexBuffer(IndexBufferRHI);
#else
				void* VoidPtr = RHILockBuffer(IndexBufferRHI, 0, sizeof(uint16) * IndexCount, RLM_WriteOnly);
				FMemory::Memcpy(VoidPtr, IndexArray, IndexCount * sizeof(uint16));
				RHIUnlockBuffer(IndexBufferRHI);
#endif
				
				ThisSharedPtr->MaskRenderBuffers.IndexBuffers.Add(DrawIter, IndexBufferRHI);
			}
		}

	});
}

const UUnLive2D* FUnLive2DRenderState::GetUnLive2D() const
{
	if (OwnerCompWeak.IsValid())
	{
		return OwnerCompWeak->GetUnLive2D();
	}
	else if (OwnerViewUIWeak.IsValid())
	{
		return OwnerViewUIWeak->GetUnLive2D();
	}

	return nullptr;
}

UMaterialInstanceDynamic* FUnLive2DRenderState::GetUnLive2DMaterial(int32 InModeIndex) const
{
	Rendering::CubismRenderer::CubismBlendMode InMode = (Rendering::CubismRenderer::CubismBlendMode )InModeIndex;
	if (!OwnerCompWeak.IsValid() && !OwnerViewUIWeak.IsValid()) return nullptr;
	UMaterialInstanceDynamic* MatDynamic = nullptr;
	switch (InMode)
	{
	case Rendering::CubismRenderer::CubismBlendMode_Normal:
	{
		if (OwnerCompWeak.IsValid())
		{
			MatDynamic = UMaterialInstanceDynamic::Create(OwnerCompWeak->UnLive2DNormalMaterial, OwnerCompWeak.Get());
			break;
		}
		
		if (OwnerViewUIWeak.IsValid())
		{
			MatDynamic = UMaterialInstanceDynamic::Create(OwnerViewUIWeak->UnLive2DNormalMaterial, OwnerViewUIWeak.Get());
			break;
		}
	}
	break;
	case Rendering::CubismRenderer::CubismBlendMode_Additive:
	{
		if (OwnerCompWeak.IsValid())
		{
			MatDynamic = UMaterialInstanceDynamic::Create(OwnerCompWeak->UnLive2DAdditiveMaterial, OwnerCompWeak.Get());
			break;
		}

		if (OwnerViewUIWeak.IsValid())
		{
			MatDynamic = UMaterialInstanceDynamic::Create(OwnerViewUIWeak->UnLive2DAdditiveMaterial, OwnerViewUIWeak.Get());
			break;
		}
	}
	break;
	case Rendering::CubismRenderer::CubismBlendMode_Multiplicative:
	{
		if (OwnerCompWeak.IsValid())
		{
			MatDynamic = UMaterialInstanceDynamic::Create(OwnerCompWeak->UnLive2DMultiplyMaterial, OwnerCompWeak.Get());
			break;
		}

		if (OwnerViewUIWeak.IsValid())
		{
			MatDynamic = UMaterialInstanceDynamic::Create(OwnerViewUIWeak->UnLive2DMultiplyMaterial, OwnerViewUIWeak.Get());
			break;
		}
	}
	break;
	}

	if (MatDynamic)
	{
		MatDynamic->AddToRoot();
	}

	return MatDynamic; 
}

FName FUnLive2DRenderState::GetDMaterialTextureParameterName() const
{
	return FName("UnLive2D");
}

void FUnLive2DRenderState::UnLive2DFillMaskParameter(CubismClippingContext* clipContext, FUnLiveMatrix& ts_MartixForMask, FUnLiveVector4& ts_BaseColor, FUnLiveVector4& ts_ChanelFlag)
{
	// チャンネル
	const csmInt32 channelNo = clipContext->_layoutChannelNo;
	// チャンネルをRGBAに変換
	Csm::Rendering::CubismRenderer::CubismTextureColor* colorChannel = UnLive2DClippingManager->GetChannelFlagAsColor(channelNo);

	csmRectF* rect = clipContext->_layoutBounds;

	ts_MartixForMask = ConvertCubismMatrix(clipContext->_matrixForMask);
	ts_BaseColor = FUnLiveVector4(rect->X * 2.0f - 1.0f, rect->Y * 2.0f - 1.0f, rect->GetRight() * 2.0f - 1.0f, rect->GetBottom() * 2.0f - 1.0f);
	ts_ChanelFlag = FUnLiveVector4(colorChannel->R, colorChannel->G, colorChannel->B, colorChannel->A);
}

void FUnLive2DRenderState::UpdateRenderBuffers(TWeakPtr<FUnLive2DRawModel> InUnLive2DRawModel)
{
	check(IsInGameThread());

	UWorld* SelfWorld = nullptr;

	if (OwnerCompWeak.IsValid())
	{
		SelfWorld = OwnerCompWeak->GetWorld();
	}
	else if (OwnerViewUIWeak.IsValid() && OwnerViewUIWeak.IsValid())
	{
		SelfWorld = OwnerViewUIWeak->GetWorld();
	}

	if (SelfWorld == nullptr) return;

	ERHIFeatureLevel::Type FeatureLevel = SelfWorld->Scene->GetFeatureLevel();

	ENQUEUE_RENDER_COMMAND(UpdateRender)([this, FeatureLevel, InUnLive2DRawModel](FRHICommandListImmediate& RHICmdList)
	{
		check(IsInRenderingThread()); // 如果不是渲染线程请弄成渲染线程

		if (GetUnLive2D() == nullptr || !InUnLive2DRawModel.IsValid()) return;


		Csm::CubismModel* UnLive2DModel = InUnLive2DRawModel.Pin()->GetModel();

		if (!UnLive2DModel->IsUsingMasking() || !UnLive2DClippingManager.IsValid()) return;

		UnLive2DClippingManager->SetupClippingContext(UnLive2DModel, this);

		if (GetUseHighPreciseMask()) return;

		UpdateMaskBufferRenderTarget(RHICmdList, UnLive2DModel, FeatureLevel);
		
	});
}

void FUnLive2DRenderState::UpdateMaskBufferRenderTarget(FRHICommandListImmediate& RHICmdList, Csm::CubismModel* tp_Model, ERHIFeatureLevel::Type FeatureLevel)
{
	if (RHICmdList.IsStalled()) return;

	FRHITexture2D* RenderTargetTexture = MaskBufferRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();

	RHICmdList.TransitionResource(ERHIAccess::WritableMask, RenderTargetTexture);

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
				/*csmFloat32 tf_Opacity = tp_Model->GetDrawableOpacity(clipDrawIndex);
				Rendering::CubismRenderer::CubismBlendMode ts_BlendMode = tp_Model->GetDrawableBlendMode(clipDrawIndex);
				csmBool tb_InvertMask = tp_Model->GetDrawableInvertedMask(clipDrawIndex);*/


				//////////////////////////////////////////////////////////////////////////
				/** Drawable draw */
				const csmInt32 td_NumVertext = tp_Model->GetDrawableVertexCount(clipDrawIndex);

				FIndexUnLiveBufferRHIRef IndexBufferRHI = MaskRenderBuffers.IndexBuffers.FindRef(clipDrawIndex);
				if (!IndexBufferRHI.IsValid()) return;

				FVertexUnLiveBufferRHIRef ScratchVertexBufferRHI = MaskRenderBuffers.VertexBuffers.FindRef(clipDrawIndex);
				if (!ScratchVertexBufferRHI.IsValid()) return;
#if UE_VERSION_OLDER_THAN(5,0,0)
				FTextureRHIRef tsr_TextureRHI = tp_Texture->Resource->TextureRHI;
#else
				FTextureRHIRef tsr_TextureRHI = tp_Texture->GetResource()->TextureRHI;
#endif

				MaskFillVertexBuffer(tp_Model, clipDrawIndex, ScratchVertexBufferRHI, RHICmdList);

				////////////////////////////////////////////////////////////////////////////
#if UE_VERSION_OLDER_THAN(5,0,0)
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
#else
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);
#endif

				FUnLiveMatrix ts_MartixForMask;
				FUnLiveVector4 ts_BaseColor;
				FUnLiveVector4 ts_ChanelFlag;

				UnLive2DFillMaskParameter(clipContext, ts_MartixForMask, ts_BaseColor, ts_ChanelFlag);

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

void FUnLive2DRenderState::MaskFillVertexBuffer(Csm::CubismModel* tp_Model, const Csm::csmInt32 drawableIndex, FVertexUnLiveBufferRHIRef ScratchVertexBufferRHI, FRHICommandListImmediate& RHICmdList)
{
	const csmInt32 td_NumVertext = tp_Model->GetDrawableVertexCount(drawableIndex);
	UE_LOG(LogUnLive2D, Verbose, TEXT("FillVertexBuffer: Vertext buffer info %d|%d >> (%u, %u)"), drawableIndex, td_NumVertext, ScratchVertexBufferRHI->GetSize(), ScratchVertexBufferRHI->GetUsage());

	if (MaskRenderBuffers.VertexCounts.Find(drawableIndex) == nullptr) return;

	//check(td_NumVertext == tp_States->VertexCount[drawableIndex]);
	{

		const csmFloat32* uvarray = reinterpret_cast<csmFloat32*>(const_cast<Live2D::Cubism::Core::csmVector2*>(tp_Model->GetDrawableVertexUvs(drawableIndex)));
		const csmFloat32* varray = const_cast<csmFloat32*>(tp_Model->GetDrawableVertices(drawableIndex));

		check(varray);
		check(uvarray);

#if UE_VERSION_OLDER_THAN(5,0,0)
		void* DrawableData = RHICmdList.LockVertexBuffer(ScratchVertexBufferRHI, 0, td_NumVertext * sizeof(FCubismVertex), RLM_WriteOnly);
#else
		void* DrawableData = RHICmdList.LockBuffer(ScratchVertexBufferRHI, 0, td_NumVertext * sizeof(FCubismVertex), RLM_WriteOnly);
#endif
		FCubismVertex* RESTRICT DestSamples = (FCubismVertex*)DrawableData;

		for (int32 td_VertexIndex = 0; td_VertexIndex < td_NumVertext; ++td_VertexIndex)
		{
			DestSamples[td_VertexIndex].Position.X = varray[td_VertexIndex * 2];
			DestSamples[td_VertexIndex].Position.Y = varray[td_VertexIndex * 2 + 1];
			DestSamples[td_VertexIndex].UV.X = uvarray[td_VertexIndex * 2];
			DestSamples[td_VertexIndex].UV.Y = uvarray[td_VertexIndex * 2 + 1];
		}

#if UE_VERSION_OLDER_THAN(5,0,0)
		RHICmdList.UnlockVertexBuffer(ScratchVertexBufferRHI);
#else
		RHICmdList.UnlockBuffer(ScratchVertexBufferRHI);
#endif
	}
}

FUnLiveMatrix FUnLive2DRenderState::GetUnLive2DPosToClipMartix(class CubismClippingContext* ClipContext, FUnLiveVector4& ChanelFlag)
{
	if (ClipContext == nullptr) return FUnLiveMatrix();

	const csmInt32 ChannelNo = ClipContext->_layoutChannelNo; // 通道

	// 将通道转换为RGBA
	Csm::Rendering::CubismRenderer::CubismTextureColor* ColorChannel = UnLive2DClippingManager->GetChannelFlagAsColor(ChannelNo);

	ChanelFlag = FUnLiveVector4(ColorChannel->R, ColorChannel->G, ColorChannel->B, ColorChannel->A);

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

