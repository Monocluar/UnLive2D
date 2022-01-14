#include "CubismSepRender.h"
#include "RHICommandList.h"
#include "TextureResource.h"
#include "ProfilingDebugging/RealtimeGPUProfiler.h"
#include "GeneratedCodeHelpers.h"
#include "RenderingThread.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "GlobalShader.h"
#include "Shader.h"
#include "RHIStaticStates.h"
#include "PipelineStateCache.h"
#include "RHIDefinitions.h"
#include "FWPort/UnLive2DRawModel.h"
#include "Templates/SharedPointer.h"
#include "RHI.h"
#include "ImageUtils.h"
#include "Engine/Texture2D.h"

#include "SceneInterface.h"
#include "FWPort/UnLive2DModelRender.h"

#include "UnLive2DAsset.h"
#include "ModelRenders.h"
#include "UnLive2D.h"
#include "CubismConfig.h"

static bool GDrawTextureForTest = false;
FAutoConsoleVariableRef CVarDrawTextureForTest(
    TEXT("cubism.db.texture"),
    GDrawTextureForTest,
    TEXT("Draw texture instead of model for debug"),
    ECVF_Default
);

static bool GCubismNoMask = false;
FAutoConsoleVariableRef CVarCubismNoMask(
    TEXT("cubism.db.nomask"),
    GCubismNoMask,
    TEXT("Draw model with no mask"),
    ECVF_Default
);

#define _DRAW_TEXTURE_ GDrawTextureForTest
#define _DRAW_MODEL_ (!_DRAW_TEXTURE_)

//////////////////////////////////////////////////////////////////////////
using Csm::csmInt32;
using Csm::csmFloat32;
using Csm::csmUint16;

struct FCompiledSeparateDraw
{
    Csm::CubismModel* InModel = nullptr;
    FCubismRenderState* InStates = nullptr;
};

//////////////////////////////////////////////////////////////////////////

static void DrawSeparateToRenderTarget_RenderThread(
    FRHICommandListImmediate& RHICmdList,
    const FCompiledSeparateDraw& CompiledSeparateDraw,
    const FName& TextureRenderTargetName,
    FTextureRenderTargetResource* OutTextureRenderTargetResource,
    ERHIFeatureLevel::Type FeatureLevel)
{


#if WANTS_DRAW_MESH_EVENTS
    FString EventName;
    TextureRenderTargetName.ToString(EventName);
    SCOPED_DRAW_EVENTF(RHICmdList, SceneCapture, TEXT("DrawSeparateToRenderTarget %s"), *EventName);
#else
    SCOPED_DRAW_EVENT(RHICmdList, DrawSeparateToRenderTarget_RenderThread);
#endif

    //////////////////////////////////////////////////////////////////////////
    check(CompiledSeparateDraw.InModel);
    Csm::CubismModel* UnLive2DModel = CompiledSeparateDraw.InModel;
    FCubismRenderState* tp_States = CompiledSeparateDraw.InStates;

    check(tp_States);
    check(IsInRenderingThread());

    /** <Mask绘制-基础版 */
    const bool tb_IsUsingMask = UnLive2DModel->IsUsingMasking();
    if (tb_IsUsingMask && tp_States->_ClippingManager.IsValid())
    {
        CubismClippingManager_UE* _clippingManager = tp_States->_ClippingManager.Get();
        _clippingManager->SetupClippingContext(*UnLive2DModel, tp_States);

        /** 如果处于低精度模式，则将遮罩绘制为单个纹理 */
        if (!tp_States->Get_UseHighPreciseMask())
        {
			FModelRenders::RenderMask_Full(tp_States, RHICmdList, _clippingManager, FeatureLevel, UnLive2DModel);
        }
    }

    /** 渲染模型主体 */
    if (_DRAW_MODEL_)
    {
        const csmInt32 drawableCount = UnLive2DModel->GetDrawableCount();
        const csmInt32* renderOrder = UnLive2DModel->GetDrawableRenderOrders();

		if (tp_States->_sortedDrawableIndexList.GetSize() < (csmUint32)drawableCount)
		{
			return;
		}

		// 按绘图顺序排序索引
		for (csmInt32 i = 0; i < drawableCount; ++i)
		{
			const csmInt32 order = renderOrder[i];

			tp_States->_sortedDrawableIndexList[order] = i;
		}


        //////////////////////////////////////////////////////////////////////////
        {
            FRHITexture2D* RenderTargetTexture = OutTextureRenderTargetResource->GetRenderTargetTexture();
            RHICmdList.TransitionResource(ERHIAccess::EWritable, RenderTargetTexture);

            FRHIRenderPassInfo RPInfo(RenderTargetTexture, ERenderTargetActions::Clear_Store, OutTextureRenderTargetResource->TextureRHI);
            RHICmdList.BeginRenderPass(RPInfo, TEXT("DrawClear"));
            RHICmdList.EndRenderPass();
        }


        //////////////////////////////////////////////////////////////////////////

        // 描画
        for (csmInt32 i = 0; i < drawableCount; ++i)
        {
            const csmInt32 drawableIndex = tp_States->_sortedDrawableIndexList[i];

            // <Drawable如果不是显示状态，则通过处理
            if (!UnLive2DModel->GetDrawableDynamicFlagIsVisible(drawableIndex))
            {
                continue;
            }

            const csmInt32 indexCount = UnLive2DModel->GetDrawableVertexIndexCount(drawableIndex);
            if (0 == indexCount)
            {
                continue;
            }

            if (tp_States->IndexBuffers.Num() == 0 || tp_States->IndexBuffers.Find(drawableIndex) == nullptr)
            {
                continue;
            }

            if (tp_States->VertexBuffers.Num() == 0 || tp_States->VertexBuffers.Find(drawableIndex) == nullptr)
            {
                UE_LOG(LogUnLive2D, Error, TEXT("DrawSeparateToRenderTarget_RenderThread: Vertext buffer not inited."));
                continue;
            }

            //////////////////////////////////////////////////////////////////////////


            //////////////////////////////////////////////////////////////////////////
            /** Mask relate */
            CubismClippingContext* clipContext = NULL;
            if (tp_States->_ClippingManager.IsValid())
            {
                csmVector<CubismClippingContext*>* ClippingContextArr = tp_States->_ClippingManager->GetClippingContextListForDraw();
                if (ClippingContextArr && drawableIndex >= 0 &&ClippingContextArr->GetSize() > (csmUint32)drawableIndex)
                {
                    clipContext = (*ClippingContextArr)[drawableIndex];
                }
            }

            const bool tb_IsMaskDraw = (nullptr != clipContext);
            if (tb_IsMaskDraw)
            {
#if !UE_BUILD_SHIPPING
                if (GCubismNoMask)
                {
                    continue;
                }
#endif

                /** High precise mask draws to mask buffer per drawable */
                if (tp_States->Get_UseHighPreciseMask())
                {
                    FModelRenders::RenderMask_Single(tp_States, RHICmdList, FeatureLevel, UnLive2DModel, clipContext);
                }

                FModelRenders::DrawSepMask(OutTextureRenderTargetResource, RHICmdList, FeatureLevel, UnLive2DModel, drawableIndex, tp_States, clipContext);
            }
            else
            {
                FModelRenders::DrawSepNormal(OutTextureRenderTargetResource, RHICmdList, FeatureLevel, UnLive2DModel, drawableIndex, tp_States);
            }


        }

    }

    if(_DRAW_TEXTURE_)
    {
        FModelRenders::DrawTestTexture(OutTextureRenderTargetResource, RHICmdList, FeatureLevel, tp_States);
    }

    tp_States->bRenderOver = true;
}


void FCubismSepRender::DrawSeparateToRenderTarget(
    class UWorld* World, 
    class UTextureRenderTarget2D* OutputRenderTarget
) const
{
    check(IsInGameThread());

    if (!OutputRenderTarget)
    {
        UE_LOG(LogUnLive2D, Error, TEXT("FCubismSepRender::DrawSeparateToRenderTarget: OutputRenderTarget not valid"));
        return;
    }

    if (!IsValid(World))
    {
        UE_LOG(LogUnLive2D, Error, TEXT("FCubismSepRender::DrawSeparateToRenderTarget: World not valid"));
        return;
    }

    if (SourceUnLive2D == nullptr|| !SourceUnLive2D->GetUnLive2DRawModel().IsValid())
    {
        UE_LOG(LogUnLive2D, Error, TEXT("FCubismSepRender::DrawSeparateToRenderTarget: _Model not valid"));
        return;
    }

    Csm::CubismModel* UnLive2DModel = SourceUnLive2D->GetUnLive2DRawModel().Pin()->GetModel();
    if (!UnLive2DModel)
    {
        UE_LOG(LogUnLive2D, Error, TEXT("FCubismSepRender::DrawSeparateToRenderTarget: GetModel not valid"));
        return;
    }

    const FName TextureRenderTargetName = OutputRenderTarget->GetFName();
    FTextureRenderTargetResource* TextureRenderTargetResource = OutputRenderTarget->GameThread_GetRenderTargetResource();

    FCompiledSeparateDraw ts_CompiledSeparateDraw;
    ts_CompiledSeparateDraw.InModel = UnLive2DModel;
    ts_CompiledSeparateDraw.InStates = &RenderStates;


    ERHIFeatureLevel::Type FeatureLevel = World->Scene->GetFeatureLevel();

    ENQUEUE_RENDER_COMMAND(CaptureCommand)(
        [ts_CompiledSeparateDraw, TextureRenderTargetResource, TextureRenderTargetName, FeatureLevel](FRHICommandListImmediate& RHICmdList)
        {
            DrawSeparateToRenderTarget_RenderThread(
                RHICmdList,
                ts_CompiledSeparateDraw,
                TextureRenderTargetName,
                TextureRenderTargetResource,
                FeatureLevel);
        }
    );
}

void FCubismSepRender::InitRender(UUnLive2D* InNewUnLive2D)
{
    if (InNewUnLive2D == nullptr) return;

    SourceUnLive2D = InNewUnLive2D;


    TWeakPtr<FUnLive2DRawModel> ModelWeakPtr = InNewUnLive2D->GetUnLive2DRawModel();

    if (!ModelWeakPtr.IsValid()) return;

    Csm::CubismModel* UnLive2DModel = ModelWeakPtr.Pin()->GetModel();
    if (!UnLive2DModel)
    {
        /** Model passing to render must be ready */
        //checkNoEntry();
        return;
    }

    csmInt32 td_DrawableCount = UnLive2DModel->GetDrawableCount();
    RenderStates._sortedDrawableIndexList.Resize(td_DrawableCount, 0);

    ENQUEUE_RENDER_COMMAND(CubismInit)(
        [=](FRHICommandListImmediate& RHICmdList)
        {
            for (csmInt32 td_DrawIter = 0; td_DrawIter < td_DrawableCount; td_DrawIter++)
            {
                /** Vertex buffer */
                const csmInt32 vcount = UnLive2DModel->GetDrawableVertexCount(td_DrawIter);
                if (0 != vcount)
                {
                    FRHIResourceCreateInfo CreateInfo_Vert;
                    void* DrawableData = nullptr;
                    FVertexBufferRHIRef ScratchVertexBufferRHI = RHICreateAndLockVertexBuffer(vcount * sizeof(FCubismVertex), BUF_Dynamic, CreateInfo_Vert, DrawableData);
                    //FMemory::Memzero(DrawableData, vcount * sizeof(FCubismVertex));

                    RHIUnlockVertexBuffer(ScratchVertexBufferRHI);

                    RenderStates.VertexBuffers.Add(td_DrawIter, ScratchVertexBufferRHI);
                    RenderStates.VertexCount.Add(td_DrawIter, vcount);
                    UE_LOG(LogUnLive2D, Log, TEXT("FCubismSepRender::InitRender: Vertext buffer info %d >> (%u, %u)"), td_DrawIter, ScratchVertexBufferRHI->GetSize(), ScratchVertexBufferRHI->GetUsage());
                }

                /** Index buffer are static */
                const csmInt32 indexCount = UnLive2DModel->GetDrawableVertexIndexCount(td_DrawIter);
                if (indexCount != 0)
                {
                    const csmUint16* indexArray = const_cast<csmUint16*>(UnLive2DModel->GetDrawableVertexIndices(td_DrawIter));

                    FRHIResourceCreateInfo CreateInfo_Indice;
                    FIndexBufferRHIRef IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), sizeof(uint16) * indexCount, BUF_Static, CreateInfo_Indice);
                    void* VoidPtr = RHILockIndexBuffer(IndexBufferRHI, 0, sizeof(uint16) * indexCount, RLM_WriteOnly);
                    FMemory::Memcpy(VoidPtr, indexArray, indexCount * sizeof(uint16));
                    RHIUnlockIndexBuffer(IndexBufferRHI);

                    RenderStates.IndexBuffers.Add(td_DrawIter, IndexBufferRHI);
                }

                UE_LOG(LogUnLive2D, Log, TEXT("FCubismSepRender::InitRender: [%d/%d] V:%d I:%d"), td_DrawIter, td_DrawableCount, vcount, indexCount);
            }
        });



    LoadTextures();

    //////////////////////////////////////////////////////////////////////////
    RenderStates._ClippingManager.Reset();
    RenderStates._ClippingManager = MakeShared<CubismClippingManager_UE>();

    RenderStates._ClippingManager->Initialize(
        *UnLive2DModel,
        UnLive2DModel->GetDrawableCount(),
        UnLive2DModel->GetDrawableMasks(),
        UnLive2DModel->GetDrawableMaskCounts()
    );

    const csmInt32 bufferHeight = RenderStates._ClippingManager->GetClippingMaskBufferSize();

    ETextureCreateFlags Flags = ETextureCreateFlags(TexCreate_None| TexCreate_RenderTargetable| TexCreate_ShaderResource);
    //Flags |= TexCreate_RenderTargetable;
    //Flags |= TexCreate_ShaderResource;
    //Flags |= TexCreate_Dynamic;
    FRHIResourceCreateInfo CreateInfo;
    CreateInfo.ClearValueBinding = FClearValueBinding(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
    RenderStates.MaskBuffer = RHICreateTexture2D(bufferHeight, bufferHeight, EPixelFormat::PF_B8G8R8A8, 1, 1, Flags, CreateInfo);
    //TransitionResource(FExclusiveDepthStencil DepthStencilMode, FRHITexture * DepthTexture)

    //////////////////////////////////////////////////////////////////////////
    RenderStates.ClearStates();
    RenderStates.RenderModelConfig = InNewUnLive2D->ModelConfigData;

    UE_LOG(LogUnLive2D, Log, TEXT("FCubismSepRender::InitRender: Completed"));
}

void FCubismSepRender::RenderThread()
{
    if (SourceUnLive2D == nullptr) return;

    TWeakPtr<FUnLive2DRawModel> ModelWeakPtr = SourceUnLive2D->GetUnLive2DRawModel();

    if (!ModelWeakPtr.IsValid()) return;

	Csm::CubismModel* UnLive2DModel = ModelWeakPtr.Pin()->GetModel();
	if (!UnLive2DModel)
	{
		/** Model passing to render must be ready */
		checkNoEntry();
		return;
	}

	csmInt32 td_DrawableCount = UnLive2DModel->GetDrawableCount();
	RenderStates._sortedDrawableIndexList.Resize(td_DrawableCount, 0);

	for (csmInt32 td_DrawIter = 0; td_DrawIter < td_DrawableCount; td_DrawIter++)
	{
		/** 顶点缓冲 */
		const csmInt32 vcount = UnLive2DModel->GetDrawableVertexCount(td_DrawIter);
		if (0 != vcount)
		{
			FRHIResourceCreateInfo CreateInfo_Vert;
			void* DrawableData = nullptr;
			FVertexBufferRHIRef ScratchVertexBufferRHI = RHICreateAndLockVertexBuffer(vcount * sizeof(FCubismVertex), BUF_Dynamic, CreateInfo_Vert, DrawableData);
			//FMemory::Memzero(DrawableData, vcount * sizeof(FCubismVertex));

			RHIUnlockVertexBuffer(ScratchVertexBufferRHI);

			RenderStates.VertexBuffers.Add(td_DrawIter, ScratchVertexBufferRHI);
			RenderStates.VertexCount.Add(td_DrawIter, vcount);
			UE_LOG(LogUnLive2D, Log, TEXT("FCubismSepRender::InitRender: Vertext buffer info %d >> (%u, %u)"), td_DrawIter, ScratchVertexBufferRHI->GetSize(), ScratchVertexBufferRHI->GetUsage());
		}

		/** Index buffer are static */
		const csmInt32 indexCount = UnLive2DModel->GetDrawableVertexIndexCount(td_DrawIter);
		if (indexCount != 0)
		{
			const csmUint16* indexArray = const_cast<csmUint16*>(UnLive2DModel->GetDrawableVertexIndices(td_DrawIter));

			FRHIResourceCreateInfo CreateInfo_Indice;
			FIndexBufferRHIRef IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), sizeof(uint16) * indexCount, BUF_Static, CreateInfo_Indice);
			void* VoidPtr = RHILockIndexBuffer(IndexBufferRHI, 0, sizeof(uint16) * indexCount, RLM_WriteOnly);
			FMemory::Memcpy(VoidPtr, indexArray, indexCount * sizeof(uint16));
			RHIUnlockIndexBuffer(IndexBufferRHI);

			RenderStates.IndexBuffers.Add(td_DrawIter, IndexBufferRHI);
		}

		UE_LOG(LogUnLive2D, Log, TEXT("FCubismSepRender::InitRender: [%d/%d] V:%d I:%d"), td_DrawIter, td_DrawableCount, vcount, indexCount);
	}
}

void FCubismSepRender::LoadTextures()
{
	if (SourceUnLive2D == nullptr) return;

	TWeakPtr<FUnLive2DRawModel> ModelWeakPtr = SourceUnLive2D->GetUnLive2DRawModel();

	if (!ModelWeakPtr.IsValid()) return;

    TWeakPtr<Csm::ICubismModelSetting> ModelSettingWeakPtr = ModelWeakPtr.Pin()->GetModelSetting();

    if (!ModelSettingWeakPtr.IsValid()) return;
    TSharedPtr<Csm::ICubismModelSetting> ModelSetting = ModelSettingWeakPtr.Pin();
    csmInt32 TextureNum = ModelSetting->GetTextureCount();

    /** Clear old textures */
    UnLoadTextures();
	RenderStates.Textures.SetNumZeroed(TextureNum);

    /** Load new texture */
    const FUnLive2DLoadData* LoadData = SourceUnLive2D->GetUnLive2DLoadData();
    if (LoadData == nullptr) return;

    for (int32 i = 0; i < LoadData->Live2DTexture2DData.Num(); i++)
    {
        UTexture2D* LoadedImage = FImageUtils::ImportBufferAsTexture2D(LoadData->Live2DTexture2DData[i].ByteData);
		if (IsValid(LoadedImage))
		{
			RenderStates.Textures[i] = LoadedImage;
		}
        LoadedImage->AddToRoot();
    }

    /** Load new texture */
    /*for (csmInt32 modelTextureNumber = 0; modelTextureNumber < td_TextureNum; modelTextureNumber++)
    {
        if (strcmp(_modelSetting->GetTextureFileName(modelTextureNumber), "") == 0)
        {
            continue;
        }

        Csm::csmString texturePath = _modelSetting->GetTextureFileName(modelTextureNumber);
        FString tstr_TempReadPath = _Model->GetHomeDir() / UTF8_TO_TCHAR(texturePath.GetRawString());

        UTexture2D* tp_LoadedImage = FImageUtils::ImportFileAsTexture2D(tstr_TempReadPath);
        if (IsValid(tp_LoadedImage))
        {
            RenderStates.Textures[modelTextureNumber] = tp_LoadedImage;
        }

        / ** Some kind of cheat, must call RemoveFromRoot as UnLoadTextures does if you directly remove some texture  * /
        tp_LoadedImage->AddToRoot();
        UE_LOG(LogUnLive2D, Log, TEXT("FCubismSepRender::LoadTextures: From %s | %d / %d >> %p"), *tstr_TempReadPath, modelTextureNumber, td_TextureNum, tp_LoadedImage);
    }*/

    UE_LOG(LogUnLive2D, Log, TEXT("FCubismSepRender::LoadTextures: Complete with %d texture"), ModelSetting->GetTextureCount());
}

void FCubismSepRender::UnLoadTextures()
{
	for (TWeakObjectPtr<UTexture2D> tp_Texture : RenderStates.Textures)
	{
		if (!tp_Texture.IsValid())
		{
            continue;
		}

		tp_Texture.Get()->RemoveFromRoot();
	}

    RenderStates.Textures.Empty();
}

FCubismSepRender::FCubismSepRender(FModelConfig& InModelConfig)
{
	RenderStates.ClearStates();
	RenderStates.RenderModelConfig = InModelConfig;
}

//////////////////////////////////////////////////////////////////////////
void FCubismRenderState::ClearStates()
{
    bNoLowPreciseMask = false;
}

bool FCubismRenderState::Get_UseHighPreciseMask() const
{
    /** Config decide not use low precise mask */
    if (!RenderModelConfig.bTryLowPreciseMask)
    {
        return true;
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
