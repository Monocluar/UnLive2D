#include "ModelRenders.h"
#include "Rendering/CubismRenderer.hpp"
#include "RHIDefinitions.h"
#include "RHIStaticStates.h"

#include "Engine/Texture2D.h"
#include "PipelineStateCache.h"

#include "UnLive2DAsset.h"

using namespace Csm;

void FModelRenders::SetUpBlendMode(
    Csm::CubismModel* tp_Model, 
    const Csm::csmInt32 drawableIndex,
    FGraphicsPipelineStateInitializer& GraphicsPSOInit
)
{
    Rendering::CubismRenderer::CubismBlendMode ts_BlendMode = tp_Model->GetDrawableBlendMode(drawableIndex);
    switch (ts_BlendMode)
    {
    case Rendering::CubismRenderer::CubismBlendMode_Normal:
    {
        GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_InverseSourceAlpha, BO_Add, BF_One, BF_InverseSourceAlpha>::GetRHI();
        break;
    }
    case Rendering::CubismRenderer::CubismBlendMode_Additive:
    {
        GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_One, BO_Add, BF_One, BF_One>::GetRHI();
        break;
    }
    case Rendering::CubismRenderer::CubismBlendMode_Multiplicative:
    {
        GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_DestColor, BF_InverseSourceAlpha, BO_Add, BF_Zero, BF_One>::GetRHI();
        break;
    }
    }
}

FMatrix FModelRenders::ConvertCubismMatrix(Csm::CubismMatrix44& InCubismMartix)
{
    FMatrix ts_Mat;

    ts_Mat.M[0][0] = InCubismMartix.GetArray()[0];
    ts_Mat.M[0][1] = InCubismMartix.GetArray()[1];
    ts_Mat.M[0][2] = InCubismMartix.GetArray()[2];
    ts_Mat.M[0][3] = InCubismMartix.GetArray()[3];

    ts_Mat.M[1][0] = InCubismMartix.GetArray()[4];
    ts_Mat.M[1][1] = InCubismMartix.GetArray()[5];
    ts_Mat.M[1][2] = InCubismMartix.GetArray()[6];
    ts_Mat.M[1][3] = InCubismMartix.GetArray()[7];

    ts_Mat.M[2][0] = InCubismMartix.GetArray()[8];
    ts_Mat.M[2][1] = InCubismMartix.GetArray()[9];
    ts_Mat.M[2][2] = InCubismMartix.GetArray()[10];
    ts_Mat.M[2][3] = InCubismMartix.GetArray()[11];

    ts_Mat.M[3][0] = InCubismMartix.GetArray()[12];
    ts_Mat.M[3][1] = InCubismMartix.GetArray()[13];
    ts_Mat.M[3][2] = InCubismMartix.GetArray()[14];
    ts_Mat.M[3][3] = InCubismMartix.GetArray()[15];

    return ts_Mat;
}



void FCubismVertexBuffer::InitRHI() 
{
    // create a static vertex buffer
    FRHIResourceCreateInfo CreateInfo;
    VertexBufferRHI = RHICreateVertexBuffer(sizeof(FCubismVertex) * 4, BUF_Static, CreateInfo);
    void* VoidPtr = RHILockVertexBuffer(VertexBufferRHI, 0, sizeof(FCubismVertex) * 4, RLM_WriteOnly);
    static const FCubismVertex Vertices[4] =
    {
        FCubismVertex(-0.9,-0.9, 0, 0),
        FCubismVertex(-0.9,+0.9, 0, 1),
        FCubismVertex(+0.9,-0.9, 1, 0),
        FCubismVertex(+0.9,+0.9, 1, 1),
    };
    FMemory::Memcpy(VoidPtr, Vertices, sizeof(FCubismVertex) * 4);
    RHIUnlockVertexBuffer(VertexBufferRHI);
}

TGlobalResource<FCubismVertexBuffer> GCubismVertexScreenBuffer;

void FCubismVertexDeclaration::InitRHI()
{
    FVertexDeclarationElementList Elements;
    uint32 Stride = sizeof(FCubismVertex);
    Elements.Add(FVertexElement(0, STRUCT_OFFSET(FCubismVertex, Position), VET_Float2, 0, Stride));
    Elements.Add(FVertexElement(0, STRUCT_OFFSET(FCubismVertex, UV), VET_Float2, 1, Stride));
    VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
}

void FCubismVertexDeclaration::ReleaseRHI()
{
    VertexDeclarationRHI.SafeRelease();
}

TGlobalResource<FCubismVertexDeclaration> GCubismVertexDeclaration;
