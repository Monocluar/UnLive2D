#include "Draw/UnLive2DSepRenderer.h"
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
#include "Model/CubismModel.hpp"
#include "Type/CubismBasicType.hpp"
#include "CubismFramework.hpp"

#if WITH_EDITOR
#include "ThumbnailRenderer/UnLive2DThumbnailRenderer.h"
#include "UnLive2DSetting.h"
#endif
#include "RHIDefinitions.h"
#include "RHICommandList.h"


FName MaskTextureParameterName = "UnLive2DMask";
FName MaskParmeterIsMeskName = "IsMask";
FName MaskParmterIsInvertedMaskName = "IsInvertedMask";
FName TintColorAndOpacityName = "TintColorAndOpacity";


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

	FUnLive2DMaskShader() {};

	FUnLive2DMaskShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		//TestFloat.Bind(Initializer.ParameterMap, TEXT("TestFloat"));
		ProjectMatrix.Bind(Initializer.ParameterMap, TEXT("ProjectMatrix"));
		BaseColor.Bind(Initializer.ParameterMap, TEXT("BaseColor"));
		ChannelFlag.Bind(Initializer.ParameterMap, TEXT("ChannelFlag"));

		MainTexture.Bind(Initializer.ParameterMap, TEXT("MainTexture"));
		MainTextureSampler.Bind(Initializer.ParameterMap, TEXT("MainTextureSampler"));
	}
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
	void SetParameters(FRHIBatchedShaderParameters& BatchedParameters,
		const FUnLiveMatrix& InProjectMatrix,
		const FUnLiveVector4& InBaseColor,
		const FUnLiveVector4& InChannelFlag,
		FTextureRHIRef ShaderResourceTexture
	)
	{
		SetShaderValue(BatchedParameters, TestFloat, 1.f);
		SetShaderValue(BatchedParameters, ProjectMatrix, InProjectMatrix);
		SetShaderValue(BatchedParameters, BaseColor, InBaseColor);
		SetShaderValue(BatchedParameters, ChannelFlag, InChannelFlag);

		SetTextureParameter(BatchedParameters, MainTexture, ShaderResourceTexture);
		SetSamplerParameter(BatchedParameters, MainTextureSampler, TStaticSamplerState<SF_Trilinear, AM_Wrap, AM_Wrap, AM_Wrap>::GetRHI());
	}
#else
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
		//SetShaderValue(RHICmdList, ShaderRHI, TestFloat, 1.f);
		SetShaderValue(RHICmdList, ShaderRHI, ProjectMatrix, InProjectMatrix);
		SetShaderValue(RHICmdList, ShaderRHI, BaseColor, InBaseColor);
		SetShaderValue(RHICmdList, ShaderRHI, ChannelFlag, InChannelFlag);

		SetTextureParameter(RHICmdList, ShaderRHI, MainTexture, ShaderResourceTexture);
		SetSamplerParameter(RHICmdList, ShaderRHI, MainTextureSampler, TStaticSamplerState<SF_Trilinear, AM_Wrap, AM_Wrap, AM_Wrap>::GetRHI());
	}
#endif

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

	FUnLive2DMaskVS() {};
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

	FUnLive2DMaskPS(){}

	/** Initialization constructor. */
	FUnLive2DMaskPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FUnLive2DMaskShader(Initializer)
	{

	}
};

IMPLEMENT_SHADER_TYPE(, FUnLive2DMaskVS, TEXT("/Plugin/UnLive2DAsset/Private/CubismMask.usf"), TEXT("MainVS"), SF_Vertex)
IMPLEMENT_SHADER_TYPE(, FUnLive2DMaskPS, TEXT("/Plugin/UnLive2DAsset/Private/CubismMask.usf"), TEXT("MainPS"), SF_Pixel)


FUnLive2DRenderState::FUnLive2DRenderState(const UUnLive2D* InNewUnLive2D, TSharedRef<FUnLive2DRawModel> InUnLive2DRawModel)
	: UnLive2DClippingManager(nullptr)
	, bNoLowPreciseMask(false)
	, SourceUnLive2D(nullptr)
	, bInitRenderBuffers(false)
	, World(nullptr)
	, UnLive2DRawModel(nullptr)
{
	InitRender(InNewUnLive2D, InUnLive2DRawModel);
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

/*
void FUnLive2DRenderState::InitRender(TWeakObjectPtr<UUnLive2D> InNewUnLive2D, TSharedPtr<FUnLive2DRawModel> InUnLive2DRawModel)
{
	if (!InNewUnLive2D.IsValid() || !InUnLive2DRawModel.IsValid()) return;

	InitRender(InNewUnLive2D.Get(), InUnLive2DRawModel);
}*/


void FUnLive2DRenderState::InitRender(const UUnLive2D* InNewUnLive2D, TSharedPtr<FUnLive2DRawModel> InUnLive2DRawModel)
{
	if (InNewUnLive2D == nullptr) return;

	if (!InUnLive2DRawModel.IsValid()) return;

	if (SourceUnLive2D.IsValid() && SourceUnLive2D.Get() == InNewUnLive2D) return;

	SourceUnLive2D = InNewUnLive2D;


	if (UnLive2DClippingManager.IsValid())
	{
		UnLive2DClippingManager.Reset();
		UnOldMaterial();
	}

	LoadTextures(InUnLive2DRawModel);

	bInitRenderBuffers = false;

	Csm::CubismModel* UnLive2DModel = InUnLive2DRawModel->GetModel();
	UnLive2DRawModel = UnLive2DModel;
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
        if (!UnLive2D->bTryLowPreciseMask)
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

void FUnLive2DRenderState::LoadTextures(TSharedPtr<FUnLive2DRawModel> InUnLive2DRawModel)
{

	const UUnLive2D* UnLive2D = GetUnLive2D();

	if (UnLive2D == nullptr) return;

	if (!InUnLive2DRawModel.IsValid()) return;

	const TWeakPtr<Csm::ICubismModelSetting> ModelSettingWeakPtr = InUnLive2DRawModel->GetModelSetting();

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

		UTexture2D* LoadedImage = SourceUnLive2D->TextureAssets[i];
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

class UTexture2D* FUnLive2DRenderState::GetRandererStatesTexturesTextureIndex(const int32& DrawableIndex) const
{

	const csmInt32 TextureIndex = UnLive2DRawModel->GetDrawableTextureIndices(DrawableIndex);
	if (!RandererStatesTextures.IsValidIndex(TextureIndex)) return nullptr;

	TWeakObjectPtr<UTexture2D> TextureWeakPtr = RandererStatesTextures[TextureIndex];

	if (!TextureWeakPtr.IsValid()) return nullptr;

	return TextureWeakPtr.Get();
}

UMaterialInstanceDynamic* FUnLive2DRenderState::GetMaterialInstanceDynamicToIndex(const Csm::CubismModel* Live2DModel, const int32 DrawableIndex, bool bIsMesk)
{

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

	auto SetMaterialInstanceDynamicParameter = [this, &Live2DModel, &DrawableIndex, &IsMeskValue, &InvertedMesk](UMaterialInstanceDynamic* DynamicMat)
	{
		UTexture2D* Texture = GetRandererStatesTexturesTextureIndex(DrawableIndex);
		//check(Texture && "Texture Is Null");
		if (Texture == nullptr)
		{
			return;
		}
		DynamicMat->SetTextureParameterValue(GetDMaterialTextureParameterName(), Texture);
		DynamicMat->SetTextureParameterValue(MaskTextureParameterName, MaskBufferRenderTarget.Get());
		DynamicMat->SetScalarParameterValue(MaskParmeterIsMeskName, IsMeskValue);
		DynamicMat->SetScalarParameterValue(MaskParmterIsInvertedMaskName, InvertedMesk);
		DynamicMat->SetVectorParameterValue(TintColorAndOpacityName, GetUnLive2D()->TintColorAndOpacity);
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

CubismClippingContext* FUnLive2DRenderState::GetClipContextInDrawableIndex(const uint32 DrawableIndex) const
{
	if (!UnLive2DClippingManager.IsValid()) return nullptr;

	csmVector<CubismClippingContext*>* ClippingContextArr = UnLive2DClippingManager->GetClippingContextListForDraw();
	if (ClippingContextArr != nullptr && DrawableIndex >= 0 && ClippingContextArr->GetSize() > DrawableIndex)
	{
		return (*ClippingContextArr)[DrawableIndex];
	}

	return nullptr;
}

void FUnLive2DRenderState::InitRenderBuffers(TSharedPtr<FUnLive2DRawModel> InUnLive2DRawModel)
{
	ENQUEUE_RENDER_COMMAND(UnLiveRenderInit)([this, InUnLive2DRawModel](FRHICommandListImmediate& RHICmdList)
	{
		check(RHICmdList.IsOutsideRenderPass());

		if (!InUnLive2DRawModel.IsValid()) return;

		Csm::CubismModel* UnLive2DModel = InUnLive2DRawModel->GetModel();

		if (UnLive2DModel == nullptr) return;

		Csm::csmInt32 DrawableCount = UnLive2DModel->GetDrawableCount();

		if (UnLive2DModel == nullptr) return;

		MaskRenderBuffers.Clear();

		for (Csm::csmInt32 DrawIter = 0; DrawIter < DrawableCount; ++DrawIter)
		{
			/** Vertex buffer */
			{
				const Csm::csmInt32 VCount = UnLive2DModel->GetDrawableVertexCount(DrawIter);
				if (VCount == 0) continue;
				FRHIResourceCreateInfo CreateInfoVert(TEXT("UnLive2DPositionBuffer"));
				void* DrawableData = nullptr;
#if ENGINE_MAJOR_VERSION < 5
				FVertexUnLiveBufferRHIRef ScratchVertexBufferRHI = RHICreateAndLockVertexBuffer(VCount * sizeof(FCubismVertex), BUF_Static, CreateInfoVert, DrawableData);
				RHIUnlockVertexBuffer(ScratchVertexBufferRHI);
#else
				//FBufferRHIRef ScratchVertexBufferRHI = RHICmdList.LockBuffer(VCount * sizeof(FCubismVertex), BUF_Static, CreateInfoVert);
				FVertexUnLiveBufferRHIRef ScratchVertexBufferRHI = RHICmdList.CreateVertexBuffer(VCount * sizeof(FCubismVertex), BUF_Static, CreateInfoVert);
				void* VoidPtr = RHICmdList.LockBuffer(ScratchVertexBufferRHI, 0, VCount * sizeof(FCubismVertex), RLM_WriteOnly);
				//FMemory::Memcpy(VoidPtr, VCount, VCount * sizeof(FCubismVertex));
				RHICmdList.UnlockBuffer(ScratchVertexBufferRHI);
#endif

				MaskRenderBuffers.VertexBuffers.Add(DrawIter, ScratchVertexBufferRHI);
				MaskRenderBuffers.VertexCounts.Add(DrawIter, VCount);
			}
			
			{
				/** Index buffer*/
				const csmInt32 IndexCount = UnLive2DModel->GetDrawableVertexIndexCount(DrawIter);

				if (IndexCount == 0) continue;

				const csmUint16* IndexArray = const_cast<csmUint16*>(UnLive2DModel->GetDrawableVertexIndices(DrawIter));
				FRHIResourceCreateInfo CreateInfoIndice(TEXT("UnLive2DIndexBuffer"));

#if ENGINE_MAJOR_VERSION < 5
				FIndexUnLiveBufferRHIRef IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), sizeof(uint16) * IndexCount, BUF_Static, CreateInfoIndice);
				void* VoidPtr = RHILockIndexBuffer(IndexBufferRHI, 0, sizeof(uint16) * IndexCount, RLM_WriteOnly);
				FMemory::Memcpy(VoidPtr, IndexArray, IndexCount * sizeof(uint16));
				RHIUnlockIndexBuffer(IndexBufferRHI);
#else
				FIndexUnLiveBufferRHIRef IndexBufferRHI = RHICmdList.CreateIndexBuffer(sizeof(uint16), sizeof(uint16) * IndexCount, BUF_Static, CreateInfoIndice);
				void* VoidPtr = RHICmdList.LockBuffer(IndexBufferRHI, 0, sizeof(uint16) * IndexCount, RLM_WriteOnly);
				FMemory::Memcpy(VoidPtr, IndexArray, IndexCount * sizeof(uint16));
				RHICmdList.UnlockBuffer(IndexBufferRHI);
#endif
				
				MaskRenderBuffers.IndexBuffers.Add(DrawIter, IndexBufferRHI);
			}
		}

	});
}

const UUnLive2D* FUnLive2DRenderState::GetUnLive2D() const
{
	return SourceUnLive2D.Get();
}

UMaterialInstanceDynamic* FUnLive2DRenderState::GetUnLive2DMaterial(int32 InModeIndex) const
{
	Rendering::CubismRenderer::CubismBlendMode InMode = (Rendering::CubismRenderer::CubismBlendMode )InModeIndex;

	UMaterialInstanceDynamic* MatDynamic = nullptr;

	switch (InMode)
	{
	case Live2D::Cubism::Framework::Rendering::CubismRenderer::CubismBlendMode_Normal:
		MatDynamic = UMaterialInstanceDynamic::Create(UnLive2DNormalMaterial, OwnerObject);
		break;
	case Live2D::Cubism::Framework::Rendering::CubismRenderer::CubismBlendMode_Additive:
		MatDynamic = UMaterialInstanceDynamic::Create(UnLive2DAdditiveMaterial, OwnerObject);
		break;
	case Live2D::Cubism::Framework::Rendering::CubismRenderer::CubismBlendMode_Multiplicative:
		MatDynamic = UMaterialInstanceDynamic::Create(UnLive2DMultiplyMaterial, OwnerObject);
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

void FUnLive2DRenderState::UnLive2DFillMaskParameter(CubismClippingContext* clipContext, FUnLiveMatrix& ts_MartixForMask, FUnLiveVector4& ts_BaseColor, FUnLiveVector4& ts_ChanelFlag) const
{
	if (!UnLive2DClippingManager.IsValid()) return;
	// チャンネル
	const csmInt32 channelNo = clipContext->_layoutChannelNo;
	// チャンネルをRGBAに変換
	Csm::Rendering::CubismRenderer::CubismTextureColor* colorChannel = UnLive2DClippingManager->GetChannelFlagAsColor(channelNo);

	csmRectF* rect = clipContext->_layoutBounds;

	ts_MartixForMask = CubismClippingManager_UE::ConvertCubismMatrix(clipContext->_matrixForMask);
	ts_BaseColor = FUnLiveVector4(rect->X * 2.0f - 1.0f, rect->Y * 2.0f - 1.0f, rect->GetRight() * 2.0f - 1.0f, rect->GetBottom() * 2.0f - 1.0f);
	ts_ChanelFlag = FUnLiveVector4(colorChannel->R, colorChannel->G, colorChannel->B, colorChannel->A);
}

bool FUnLive2DRenderState::UnLive2DFillMaskParameter(const uint32 DrawableIndex, FUnLiveMatrix& ts_MartixForMask, FUnLiveVector4& ts_BaseColor, FUnLiveVector4& ts_ChanelFlag) const
{
	if (CubismClippingContext* ClipContext = GetClipContextInDrawableIndex(DrawableIndex))
	{
		UnLive2DFillMaskParameter(ClipContext, ts_MartixForMask, ts_BaseColor, ts_ChanelFlag);
	}

	return false;
}

int32 UpDataIndex = 0;

void FUnLive2DRenderState::UpdateRenderBuffers(TWeakPtr<FUnLive2DRawModel> InUnLive2DRawModel)
{
	return;
	if (World == nullptr) return;

	ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();

	if (!bInitRenderBuffers)
	{
		InitRenderBuffers(InUnLive2DRawModel.Pin());
		bInitRenderBuffers = true;
	}
	else
	{
	
		ENQUEUE_RENDER_COMMAND(UpdateRender)([this, FeatureLevel, InUnLive2DRawModel](FRHICommandListImmediate& RHICmdList)
		{
			check(RHICmdList.IsOutsideRenderPass());

			if (GetUnLive2D() == nullptr || !InUnLive2DRawModel.IsValid()) return;

			if (GetUseHighPreciseMask()) return;

			Csm::CubismModel* UnLive2DModel = InUnLive2DRawModel.Pin()->GetModel();

			if (!UnLive2DModel->IsUsingMasking() || !UnLive2DClippingManager.IsValid()) return;

			UnLive2DClippingManager->SetupClippingContext(UnLive2DModel, this);


			UpdateMaskBufferRenderTarget(RHICmdList, UnLive2DModel, FeatureLevel);

		});
	}

}

void FUnLive2DRenderState::UpdateMaskBufferRenderTarget(FRHICommandListImmediate& RHICmdList, Csm::CubismModel* tp_Model, ERHIFeatureLevel::Type FeatureLevel)
{
	if (RHICmdList.IsStalled()) return;
#if ENGINE_MAJOR_VERSION >=5 && ENGINE_MINOR_VERSION > 1
	const FTextureRHIRef RenderTargetTexture = MaskBufferRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
#else
	FRHITexture2D* RenderTargetTexture = MaskBufferRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
#endif

	RHICmdList.Transition(FRHITransitionInfo(RenderTargetTexture, ERHIAccess::Unknown, ERHIAccess::WritableMask));

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
#if ENGINE_MAJOR_VERSION >=5 && ENGINE_MINOR_VERSION > 4
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, ERasterizerDepthClipMode::DepthClip, true>::GetRHI();
#else
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, true, true>::GetRHI();
#endif
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

				TWeakObjectPtr<UTexture2D> tp_Texture = GetRandererStatesTexturesTextureIndex(clipDrawIndex);

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
#if ENGINE_MAJOR_VERSION < 5
				FTextureRHIRef tsr_TextureRHI = tp_Texture->Resource->TextureRHI;
#else
				FTextureRHIRef tsr_TextureRHI = tp_Texture->GetResource()->TextureRHI;
#endif

				MaskFillVertexBuffer(tp_Model, clipDrawIndex, ScratchVertexBufferRHI, RHICmdList);

				////////////////////////////////////////////////////////////////////////////
#if ENGINE_MAJOR_VERSION < 5
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
#else
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);
#endif

				FUnLiveMatrix ts_MartixForMask;
				FUnLiveVector4 ts_BaseColor;
				FUnLiveVector4 ts_ChanelFlag;

				UnLive2DFillMaskParameter(clipContext, ts_MartixForMask, ts_BaseColor, ts_ChanelFlag);
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
				SetShaderParametersLegacyVS(RHICmdList, VertexShader, ts_MartixForMask, ts_BaseColor, ts_ChanelFlag, tsr_TextureRHI);
				SetShaderParametersLegacyPS(RHICmdList, PixelShader, ts_MartixForMask, ts_BaseColor, ts_ChanelFlag, tsr_TextureRHI);
#else
				VertexShader->SetParameters(RHICmdList, VertexShader.GetVertexShader(), ts_MartixForMask, ts_BaseColor, ts_ChanelFlag, tsr_TextureRHI);
				PixelShader->SetParameters(RHICmdList, PixelShader.GetPixelShader(), ts_MartixForMask, ts_BaseColor, ts_ChanelFlag, tsr_TextureRHI);
#endif

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

void FUnLive2DRenderState::InitRHI(FRHICommandListImmediate& RHICmdList, ERHIFeatureLevel::Type InFeatureLevel)
{
	UnLive2DClippingManager->SetupClippingContext(UnLive2DRawModel, this);
#if ENGINE_MAJOR_VERSION >=5 && ENGINE_MINOR_VERSION > 1
	const FTextureRHIRef RenderTargetTexture = MaskBufferRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
#else
	FRHITexture2D* RenderTargetTexture = MaskBufferRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
#endif

	RHICmdList.Transition(FRHITransitionInfo(RenderTargetTexture, ERHIAccess::Unknown, ERHIAccess::WritableMask));

	FRHIRenderPassInfo RPInfo(RenderTargetTexture, ERenderTargetActions::Clear_Store, RenderTargetTexture);
	RHICmdList.BeginRenderPass(RPInfo, TEXT("DrawMask01"));
	{
		float MaskSize = UnLive2DClippingManager->_clippingMaskBufferSize;

		RHICmdList.SetViewport(0.f, 0.f, 0.f, MaskSize, MaskSize, 1.f);

		FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(InFeatureLevel);
		TShaderMapRef< FUnLive2DMaskVS > VertexShader(GlobalShaderMap);
		TShaderMapRef< FUnLive2DMaskPS > PixelShader(GlobalShaderMap);

		// Set the graphic pipeline state.
		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
		//GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_Zero, BF_InverseSourceColor, BO_Add, BF_Zero, BF_InverseSourceAlpha>::GetRHI();
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_InverseSourceAlpha, BO_Add, BF_One, BF_InverseSourceAlpha>::GetRHI();
#if ENGINE_MAJOR_VERSION >=5 && ENGINE_MINOR_VERSION > 4
		//GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, ERasterizerDepthClipMode::DepthClip, true>::GetRHI();
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
#else
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, true, true>::GetRHI();
#endif
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
				if (!UnLive2DRawModel->GetDrawableDynamicFlagVertexPositionsDidChange(clipDrawIndex)) continue;

				// TODO: push [IsCulling] to [RasterizerState] if needed
				const bool tb_IsCulling = (UnLive2DRawModel->GetDrawableCulling(clipDrawIndex) != 0);

				TWeakObjectPtr<UTexture2D> tp_Texture = GetRandererStatesTexturesTextureIndex(clipDrawIndex);

				if (tp_Texture == nullptr)
				{
					UE_LOG(LogUnLive2D, Warning, TEXT("DrawSeparateToRenderTarget_RenderThread:[Mask] Texture  invalid"));
					continue;
				}


				const csmInt32 indexCount = UnLive2DRawModel->GetDrawableVertexIndexCount(clipDrawIndex);
				if (0 == indexCount) continue;

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
				const csmInt32 td_NumVertext = UnLive2DRawModel->GetDrawableVertexCount(clipDrawIndex);

				FIndexUnLiveBufferRHIRef IndexBufferRHI = MaskRenderBuffers.IndexBuffers.FindRef(clipDrawIndex);
				if (!IndexBufferRHI.IsValid()) return;

				FVertexUnLiveBufferRHIRef ScratchVertexBufferRHI = MaskRenderBuffers.VertexBuffers.FindRef(clipDrawIndex);
				if (!ScratchVertexBufferRHI.IsValid()) return;
#if ENGINE_MAJOR_VERSION < 5
				FTextureRHIRef tsr_TextureRHI = tp_Texture->Resource->TextureRHI;
#else
				FTextureRHIRef tsr_TextureRHI = tp_Texture->GetResource()->TextureRHI;
#endif

				MaskFillVertexBuffer(UnLive2DRawModel, clipDrawIndex, ScratchVertexBufferRHI, RHICmdList);

				////////////////////////////////////////////////////////////////////////////
#if ENGINE_MAJOR_VERSION < 5
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
#else
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);
#endif

				FUnLiveMatrix ts_MartixForMask;
				FUnLiveVector4 ts_BaseColor;
				FUnLiveVector4 ts_ChanelFlag;

				UnLive2DFillMaskParameter(clipContext, ts_MartixForMask, ts_BaseColor, ts_ChanelFlag);
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
				SetShaderParametersLegacyVS(RHICmdList, VertexShader, ts_MartixForMask, ts_BaseColor, ts_ChanelFlag, tsr_TextureRHI);
				SetShaderParametersLegacyPS(RHICmdList, PixelShader, ts_MartixForMask, ts_BaseColor, ts_ChanelFlag, tsr_TextureRHI);
#else
				VertexShader->SetParameters(RHICmdList, VertexShader.GetVertexShader(), ts_MartixForMask, ts_BaseColor, ts_ChanelFlag, tsr_TextureRHI);
				PixelShader->SetParameters(RHICmdList, PixelShader.GetPixelShader(), ts_MartixForMask, ts_BaseColor, ts_ChanelFlag, tsr_TextureRHI);
#endif

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

void FUnLive2DRenderState::ReleaseResource()
{

}

void FUnLive2DRenderState::MaskFillVertexBuffer(Csm::CubismModel* tp_Model, const int32 drawableIndex, FVertexUnLiveBufferRHIRef ScratchVertexBufferRHI, FRHICommandListImmediate& RHICmdList)
{
	const csmInt32 td_NumVertext = tp_Model->GetDrawableVertexCount(drawableIndex);
	UE_LOG(LogUnLive2D, Verbose, TEXT("FillVertexBuffer: Vertext buffer info %d|%d >> (%u, %u)"), drawableIndex, td_NumVertext, ScratchVertexBufferRHI->GetSize(), ScratchVertexBufferRHI->GetUsage());

	if (MaskRenderBuffers.VertexCounts.Find(drawableIndex) == nullptr) return;

	//check(td_NumVertext == tp_States->VertexCount[drawableIndex]);
	{

		const Live2D::Cubism::Core::csmVector2* uvarray = tp_Model->GetDrawableVertexUvs(drawableIndex);
		const csmFloat32* varray = const_cast<csmFloat32*>(tp_Model->GetDrawableVertices(drawableIndex));

		check(varray);
		check(uvarray);

#if ENGINE_MAJOR_VERSION < 5
		void* DrawableData = RHICmdList.LockVertexBuffer(ScratchVertexBufferRHI, 0, td_NumVertext * sizeof(FCubismVertex), RLM_WriteOnly);
#else
		void* DrawableData = RHICmdList.LockBuffer(ScratchVertexBufferRHI, 0, td_NumVertext * sizeof(FCubismVertex), RLM_WriteOnly);
#endif
		FCubismVertex* RESTRICT DestSamples = (FCubismVertex*)DrawableData;

		for (int32 td_VertexIndex = 0; td_VertexIndex < td_NumVertext; ++td_VertexIndex)
		{
			DestSamples[td_VertexIndex].Position.X = varray[td_VertexIndex * 2];
			DestSamples[td_VertexIndex].Position.Y = varray[td_VertexIndex * 2 + 1];
			DestSamples[td_VertexIndex].UV.X = uvarray[td_VertexIndex].X;
			DestSamples[td_VertexIndex].UV.Y = uvarray[td_VertexIndex].Y;
		}

#if ENGINE_MAJOR_VERSION < 5
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

	return CubismClippingManager_UE::ConvertCubismMatrix(ClipContext->_matrixForDraw);
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

void FUnLive2DRenderState::SetUnLive2DMaterial(int32 InModeIndex, UMaterialInterface* NewUnLive2DMaterial)
{
	Rendering::CubismRenderer::CubismBlendMode InMode = (Rendering::CubismRenderer::CubismBlendMode)InModeIndex;
	switch (InMode)
	{
	case Live2D::Cubism::Framework::Rendering::CubismRenderer::CubismBlendMode_Normal:
		UnLive2DNormalMaterial = NewUnLive2DMaterial;
		break;
	case Live2D::Cubism::Framework::Rendering::CubismRenderer::CubismBlendMode_Additive:
		UnLive2DAdditiveMaterial = NewUnLive2DMaterial;
		break;
	case Live2D::Cubism::Framework::Rendering::CubismRenderer::CubismBlendMode_Multiplicative:
		UnLive2DMultiplyMaterial = NewUnLive2DMaterial;
		break;
	}
}

void FUnLive2DRenderState::SetUnLive2D(class UUnLive2D* InUnLive2D)
{
	SourceUnLive2D = InUnLive2D;
}

