// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"
#include "CubismFramework.hpp"
#include "Type/csmVector.hpp"
#include "CubismConfig.h"
#include "RHIResources.h"


class UTexture2D;

struct FCubismRenderState
{
    Csm::csmVector<Csm::csmInt32> _sortedDrawableIndexList;       ///< 按绘图顺序排列绘图对象的索引的列表

    TMap<int32, FIndexBufferRHIRef> IndexBuffers;
    TMap<int32, FVertexBufferRHIRef> VertexBuffers;
    TMap<int32, int32> VertexCount;

    TArray<TWeakObjectPtr<UTexture2D>> Textures;

    TSharedPtr<class CubismClippingManager_UE> _ClippingManager;

    FTexture2DRHIRef MaskBuffer;

    FModelConfig RenderModelConfig;

    bool bIsRendering = false;
    bool bRenderOver = false;

public:
    void ClearStates();

    void NoLowPreciseMask(bool val) { bNoLowPreciseMask = val; }
    bool Get_UseHighPreciseMask() const;

private:
    bool bNoLowPreciseMask = false;
};

/** 
 * 
 */
struct FCubismSepRender
{ 
public:

	void DrawSeparateToRenderTarget(
		class UWorld* World,
		class UTextureRenderTarget2D* OutputRenderTarget) const;

    void InitRender(class UUnLive2D* InNewUnLive2D);

public:
    void RenderThread();

private:
    void LoadTextures();

    void UnLoadTextures();

public:
    FCubismSepRender(){}

    FCubismSepRender(FModelConfig& InModelConfig);

private:

    UUnLive2D* SourceUnLive2D;

public:
    /** TODO: We are cheating */
    mutable FCubismRenderState RenderStates;

};