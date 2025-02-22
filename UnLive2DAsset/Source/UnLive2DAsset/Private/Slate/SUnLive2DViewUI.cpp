#include "Slate/SUnLive2DViewUI.h"
#include "UnLive2D.h"
#include "Templates/SharedPointer.h"
#include "Model/CubismModel.hpp"
#include "Styling/SlateBrush.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Framework/Application/SlateApplication.h"
#include "FWPort/UnLive2DRawModel.h"
#include "FWPort/UnLive2DModelRender.h"
#include "Rendering/DrawElements.h"
#include "Engine/TextureRenderTarget2D.h"

static int32 UnLive2DBrushNameId = 0;


void SUnLive2DViewUI::Construct(const FArguments& InArgs, const UUnLive2D* InUnLive2D)
{
	bCombinedbBatch = true;
	PlayRate = InArgs._PlayRate;
	RenderTargetSize = InArgs._RenderTargetSize;
	UnLive2DRenderType = InArgs._UnLive2DRenderType;
	NormalMaterial = InArgs._NormalMaterial;
	AdditiveMaterial = InArgs._AdditiveMaterial;
	MultiplyMaterial = InArgs._MultiplyMaterial;
	RTMaterial = InArgs._RTMaterial;
	SetUnLive2D(InUnLive2D);
}

SUnLive2DViewUI::~SUnLive2DViewUI()
{
	ClearRTCache();
}

void SUnLive2DViewUI::SetUnLive2D(const UUnLive2D* InUnLive2D)
{
	if (SourceUnLive2DPtr.IsValid() && SourceUnLive2DPtr.Get() == InUnLive2D) return;
	SourceUnLive2DPtr = InUnLive2D;
	InitUnLive2D();
}

void SUnLive2DViewUI::SetPlayRate(const TAttribute<float>& InValueAttribute)
{
	PlayRate = InValueAttribute;
}

void SUnLive2DViewUI::SetLive2DRenderType(EUnLive2DRenderType InUnLive2DRenderType)
{
	if (UnLive2DRenderType == InUnLive2DRenderType) return;
	UnLive2DRenderType = InUnLive2DRenderType;
	InitLive2DRenderType();
}

void SUnLive2DViewUI::InitUnLive2D()
{
	if (!FSlateApplication::IsInitialized()) return;

	if (!SourceUnLive2DPtr.IsValid()) return;

	const UUnLive2D* SourceUnLive2D = SourceUnLive2DPtr.Get();

	if (!UnLive2DRawModel.IsValid())
	{
		UnLive2DRawModel = SourceUnLive2D->CreateLive2DRawModel();
		UnLive2DRawModel->OnMotionPlayEnd.BindSP(this, &SUnLive2DViewUI::OnMotionPlayeEnd);
	}
	else
	{
		if (UnLive2DRawModel->GetOwnerLive2D().IsValid() && UnLive2DRawModel->GetOwnerLive2D().Get() != SourceUnLive2D)
		{
			UnLive2DRawModel.Reset();
			UnLive2DRawModel = SourceUnLive2D->CreateLive2DRawModel();
			UnLive2DRawModel->OnMotionPlayEnd.BindSP(this, &SUnLive2DViewUI::OnMotionPlayeEnd);
		}
	}
	CreateClippingManager();
	InitLive2DRenderType();
}

void SUnLive2DViewUI::InitLive2DRenderType()
{
	ClearRTCache();
	if (UnLive2DRenderType == EUnLive2DRenderType::RenderTarget)
	{
		RenderTarget = NewObject<UTextureRenderTarget2D>(GetTransientPackage());
		RenderTarget->ClearColor = FLinearColor(0.f, 0.f, 0.f, 0.f);
		const EPixelFormat RequestedFormat = FSlateApplication::Get().GetRenderer()->GetSlateRecommendedColorFormat();
		RenderTarget->InitCustomFormat(RenderTargetSize, RenderTargetSize, RequestedFormat, false);
		RenderTarget->AddToRoot();
	}

	if (!UnLive2DClippingManager.IsValid()) return;
	const csmInt32 BufferHeight = UnLive2DClippingManager->GetClippingMaskBufferSize();
	if (UnLive2DRenderType == EUnLive2DRenderType::Mesh)
	{
		MaskBufferRenderTarget = NewObject<UTextureRenderTarget2D>(GetTransientPackage());
		MaskBufferRenderTarget->ClearColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
		MaskBufferRenderTarget->InitCustomFormat(BufferHeight, BufferHeight, EPixelFormat::PF_B8G8R8A8, false);
		MaskBufferRenderTarget->AddToRoot();
	}
	else
	{
		ETextureCreateFlags Flags = ETextureCreateFlags(TexCreate_None | TexCreate_RenderTargetable | TexCreate_ShaderResource);
#if ENGINE_MAJOR_VERSION >= 5
		const FRHITextureCreateDesc Desc = FRHITextureCreateDesc::Create2D(TEXT("FUnLive2DTargetBoxProxy_UpdateSection_RenderThread"), BufferHeight, BufferHeight, PF_R8G8B8A8)
			.SetFlags(Flags).SetClearValue(FClearValueBinding(FLinearColor::White));

		MaskBuffer = RHICreateTexture(Desc);
#else
		FRHIResourceCreateInfo CreateInfo(TEXT("FUnLive2DTargetBoxProxy_UpdateSection_RenderThread"));
		CreateInfo.ClearValueBinding = FClearValueBinding(FLinearColor::White);
		MaskBuffer = RHICreateTexture2D(BufferHeight, BufferHeight, PF_R8G8B8A8, 1, 1, Flags, CreateInfo);
#endif
	}
}

void SUnLive2DViewUI::ClearRTCache()
{
	if (UnLive2DRenderType == EUnLive2DRenderType::Mesh)
	{
		if (MaskBufferRenderTarget.IsValid())
		{
			MaskBufferRenderTarget->RemoveFromRoot();
			MaskBufferRenderTarget.Reset();
		}
	}
	else
	{
		MaskBuffer.SafeRelease();
		if (IsValid(RenderTarget))
		{
			RenderTarget->RemoveFromRoot();
			RenderTarget = nullptr;
		}
	}
	for (auto& Item : UnLive2DToBlendMaterialList)
	{
		if (Item.Value->UnLive2DRenderType == EUnLive2DRenderType::RenderTarget ) continue;
		UMaterialInstanceDynamic* InstanceDynamic = Cast<UMaterialInstanceDynamic>(Item.Value->GetResourceObject());
		InstanceDynamic->RemoveFromRoot();
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 4
		InstanceDynamic->OnRemovedAsOverride(OwnerComponent.Get());
#endif
	}
	UnLive2DToBlendMaterialList.Empty();

	UnLive2DCustomVertsData.Empty();
	ClearRenderBaseData();
}

bool SUnLive2DViewUI::UpdataSections()
{
	TArray<uint16> SortedDrawableIndexList;
	if (!UpDataDrawableIndexList(SortedDrawableIndexList)) return false;
	UnLive2DCustomVertsData.Empty();
	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel->GetModel();
	for (const uint16& DrawableIndex : SortedDrawableIndexList)
	{
		if (!GetDrawableDynamicIsVisible(DrawableIndex)) continue;

		const csmInt32 TextureIndex = UnLive2DModel->GetDrawableTextureIndices(DrawableIndex);

		uint8 MaskID;
		uint8 Flags = GetUnLive2DShaderFlagsByDrawableIndex(DrawableIndex, MaskID);
		FCustomVertsData* MeshSectionData = nullptr;
		if (!bCombinedbBatch || UnLive2DCustomVertsData.Num() == 0 || !UnLive2DCustomVertsData.Top().Equals(Flags, TextureIndex, MaskID))
		{
			MeshSectionData = &UnLive2DCustomVertsData.AddDefaulted_GetRef();
			MeshSectionData->Flags = Flags;
			MeshSectionData->MaskUID = MaskID;
			MeshSectionData->TextureIndex = TextureIndex;
			MeshSectionData->Brush = MakeSlateBrushByFlags(Flags, TextureIndex);
		}
		else
			MeshSectionData = &UnLive2DCustomVertsData.Top();

		MeshSectionData->DrawableCounts.Add(DrawableIndex);
	}
	return true;
}

void SUnLive2DViewUI::Flush(int32 LayerId, FSlateWindowElementList& OutDrawElements, bool bParentEnabled) const
{
	const bool bIsEnabled = ShouldBeEnabled(bParentEnabled);

	if (UnLive2DRenderType == EUnLive2DRenderType::Mesh)
	{
		const ESlateDrawEffect DrawEffects = bIsEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
		for (int32 i = 0; i < UnLive2DCustomVertsData.Num(); i++)
		{
			//TSharedPtr<FSlateBrush> Brush = MakeShareable(new FUnLive2DSlateMaterialBrush(UnLive2DCustomVertsData[i].Brush, FVector2D(64.f, 64.f)));
			FSlateResourceHandle RenderingResourceHandle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*UnLive2DCustomVertsData[i].Brush);

			FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, RenderingResourceHandle, UnLive2DCustomVertsData[i].InterlottingLive2DVertexData, UnLive2DCustomVertsData[i].InterlottingLive2DIndexData, nullptr, 0, 0, DrawEffects);
		}
	}

}

void SUnLive2DViewUI::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SLeafWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (UnLive2DRawModel.IsValid())
	{
		UnLive2DRawModel->OnUpDate(InDeltaTime * PlayRate.Get());
		//UpdateRenderer();
	}
}

TSharedPtr<FSlateBrush> SUnLive2DViewUI::MakeSlateBrushByFlags(const uint8& Flags, const int32& InTextureIndex)
{
	FSoftObjectPath UnLive2DMaterialSoftObject;
	uint16 MapIndex = 0;
	if (UnLive2DRenderType == EUnLive2DRenderType::Mesh)
	{
		int32 BlendMode = 0;

		if (Flags & EUnLive2DShaderFlags::BlendMode_Add)
		{
			BlendMode = 1;
			UnLive2DMaterialSoftObject = AdditiveMaterial;
		}
		else if (Flags & EUnLive2DShaderFlags::BlendMode_Mul)
		{
			BlendMode = 1;
			UnLive2DMaterialSoftObject = MultiplyMaterial;
		}
		else
			UnLive2DMaterialSoftObject = NormalMaterial;
		uint16 BlendModeIndex = BlendMode * 100;
		MapIndex = BlendModeIndex + InTextureIndex;

	}

	TSharedRef<FUnLive2DSlateMaterialBrush> const* FindMaterial = UnLive2DToBlendMaterialList.Find(MapIndex);

	if (FindMaterial)  return *FindMaterial;
	if (UnLive2DRenderType == EUnLive2DRenderType::RenderTarget)
	{
		TSharedPtr<FUnLive2DSlateMaterialBrush> SlateBrush = MakeShareable(new FUnLive2DSlateMaterialBrush(RenderTarget));
		UnLive2DToBlendMaterialList.Add(MapIndex, SlateBrush.ToSharedRef());
		return SlateBrush;
	}

	if (UnLive2DMaterialSoftObject.IsNull()) return nullptr;
	if (UMaterialInterface* MaterialInterface = Cast<UMaterialInterface>(UnLive2DMaterialSoftObject.TryLoad()))
	{
		UUnLive2D* Owner = const_cast<UUnLive2D*>(SourceUnLive2DPtr.Get()) ;
		if (Owner == nullptr || !Owner->TextureAssets.IsValidIndex(InTextureIndex)) return nullptr;
		UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(MaterialInterface, Owner);
		Material->SetTextureParameterValue(TEXT("UnLive2D"), Owner->TextureAssets[InTextureIndex]);
		if (MaskBufferRenderTarget.IsValid())
		{
			Material->SetTextureParameterValue(TEXT("UnLive2DMask"), MaskBufferRenderTarget.Get());
		}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 4
		Material->OnAssignedAsOverride(Owner);
		Material->AddToCluster(Owner, true);
#endif
		Material->AddToRoot();
		TSharedPtr<FUnLive2DSlateMaterialBrush> SlateBrush = MakeShareable(new FUnLive2DSlateMaterialBrush(Material, FVector2D(64.f, 64.f)));
		UnLive2DToBlendMaterialList.Add(MapIndex, SlateBrush.ToSharedRef());
		return  SlateBrush;
	}
	return nullptr;
}

const UTexture2D* SUnLive2DViewUI::GetTexture(const uint8& InTextureIndex) const
{
	if (!SourceUnLive2DPtr.IsValid() || !SourceUnLive2DPtr->TextureAssets.IsValidIndex(InTextureIndex)) return nullptr;
	return SourceUnLive2DPtr->TextureAssets[InTextureIndex];
}

int32 SUnLive2DViewUI::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (!UnLive2DRawModel.IsValid()) return LayerId;

	FLinearColor WidgetStyleColor = InWidgetStyle.GetColorAndOpacityTint();
	if (WidgetStyleColor.A <=0) return LayerId;

	TSharedRef<SUnLive2DViewUI> ThisPtr = ConstCastSharedRef<SUnLive2DViewUI>(SharedThis(this));
	TSharedPtr<CubismClippingManager_UE> ClippingManager = ThisPtr->UnLive2DClippingManager;
	ENQUEUE_RENDER_COMMAND(SUnLive2DViewUI_OnPaint)([ThisPtr,ClippingManager](FRHICommandListImmediate& RHICmdList)
	{
		if (ClippingManager.IsValid())
		{
			bool bNoLowPreciseMask = false;
			ClippingManager->SetupClippingContext(bNoLowPreciseMask);
			ClippingManager->RenderMask_Full(RHICmdList, GMaxRHIFeatureLevel, ThisPtr->GetMaskTextureRHIRef());
		}

		if (ThisPtr->UnLive2DRenderType == EUnLive2DRenderType::RenderTarget)
		{
			if (ThisPtr->RenderTarget == nullptr) return;
			if (ThisPtr->UpdataRTSections(ThisPtr->bCombinedbBatch))
			{
				ThisPtr->DrawSeparateToRenderTarget_RenderThread(RHICmdList, ThisPtr->RenderTarget->GetRenderTargetResource(), GMaxRHIFeatureLevel, ThisPtr->MaskBuffer);
			}
		}
	});

	if (UnLive2DRenderType == EUnLive2DRenderType::Mesh)
	{
		ThisPtr->UpdataSections();
		for (FCustomVertsData& Item : ThisPtr->UnLive2DCustomVertsData)
		{
			bool bCreateIndexData = Item.InterlottingLive2DIndexData.Num() > 0;
			if (bCreateIndexData && !IsCombinedbBatchDidChange(Item.DrawableCounts)) continue;
			Item.UpDataVertsData(AllottedGeometry, UnLive2DRawModel, UnLive2DClippingManager);
		}

		// 描画
		ThisPtr->Flush(LayerId, OutDrawElements, bParentEnabled);
	}
	else
	{
		const ESlateDrawEffect DrawEffects = ShouldBeEnabled(bParentEnabled) ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
		const FLinearColor FinalColorAndOpacity(InWidgetStyle.GetColorAndOpacityTint() * SourceUnLive2DPtr->TintColorAndOpacity);
		TSharedPtr<FSlateBrush> SlateBrush = ThisPtr->MakeSlateBrushByFlags(0,0);
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), SlateBrush.Get(), DrawEffects, FinalColorAndOpacity);

	}

	return LayerId;
}

FVector2D SUnLive2DViewUI::ComputeDesiredSize(float) const
{
	return FVector2D(64.f, 64.f);
}

void SUnLive2DViewUI::OnMotionPlayeEnd()
{

}

FTextureRHIRef SUnLive2DViewUI::GetMaskTextureRHIRef() const
{
	if (UnLive2DRenderType == EUnLive2DRenderType::Mesh)
	{
		if (!MaskBufferRenderTarget.IsValid()) return FTextureRHIRef();
#if ENGINE_MAJOR_VERSION >=5 && ENGINE_MINOR_VERSION > 1
		const FTextureRHIRef RenderTargetTexture = MaskBufferRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
#else
		FRHITexture2D* RenderTargetTexture = MaskBufferRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
#endif
		return RenderTargetTexture;
	}
	else
		return MaskBuffer;
}

void SUnLive2DViewUI::FCustomVertsData::UpDataVertsData(const FGeometry& AllottedGeometry,TWeakPtr<FUnLive2DRawModel> InUnLive2DRawModel, TWeakPtr<CubismClippingManager_UE> InUnLive2DClippingManager)
{
	if (!InUnLive2DRawModel.IsValid()) return;
	bool bCreateIndexData = InterlottingLive2DIndexData.Num() > 0;
	uint16 VerticesIndex = 0;

	Csm::CubismModel* UnLive2DModel = InUnLive2DRawModel.Pin()->GetModel();
	FULVector2f BoundsSize = FULVector2f(UnLive2DModel->GetCanvasWidth(), UnLive2DModel->GetCanvasHeight());

	const FULVector2f WidgetSize = AllottedGeometry.GetLocalSize();

	const FULVector2f WidgetScale = WidgetSize / BoundsSize;
	const float SetupScale = WidgetScale.GetMin();
	const FSlateRenderTransform& Transform = AllottedGeometry.GetAccumulatedRenderTransform();

	bool bMask = Flags & EUnLive2DShaderFlags::Mesk;
	for (const int32& DrawableIndex : DrawableCounts)
	{
		if (!bCreateIndexData)
		{
			const csmInt32 VertexIndexCount = UnLive2DModel->GetDrawableVertexIndexCount(DrawableIndex); // 获得Drawable的顶点索引个数
			const csmUint16* IndicesArray = UnLive2DModel->GetDrawableVertexIndices(DrawableIndex); //顶点索引
			for (int32 Index = 0; Index < VertexIndexCount; ++Index)
			{
				InterlottingLive2DIndexData.Add(VerticesIndex + IndicesArray[Index]);
			}
		}

		csmFloat32 Opacity = UnLive2DModel->GetDrawableOpacity(DrawableIndex); // 获取不透明度

		const csmInt32 NumVertext = UnLive2DModel->GetDrawableVertexCount(DrawableIndex); // 获得Drawable顶点的个数
		const csmFloat32* VertexArray = UnLive2DModel->GetDrawableVertices(DrawableIndex); // 顶点组
		const  Live2D::Cubism::Core::csmVector2* UVArray = UnLive2DModel->GetDrawableVertexUvs(DrawableIndex); // 获取UV组

		FUnLiveMatrix MartixForDraw;
		FUnLiveVector4 ts_BaseColor;
		FColor ClipColor = FColor::White;
		if (bMask && InUnLive2DClippingManager.IsValid())
		{
			InUnLive2DClippingManager.Pin()->GetFillMaskMartixForMask(DrawableIndex, MartixForDraw, ts_BaseColor);
			if (const CubismRenderer::CubismTextureColor* ChanelFlagColor = InUnLive2DClippingManager.Pin()->GetChannelFlagAsColorByDrawableIndex(DrawableIndex))
				ClipColor = FColor(ChanelFlagColor->R * 255, ChanelFlagColor->G * 255, ChanelFlagColor->B * 255, ChanelFlagColor->A * 255);
			
		}
		bool bInvertedMesk = UnLive2DModel->GetDrawableInvertedMask(DrawableIndex);
		for (int32 VertexIndex = 0; VertexIndex < NumVertext; ++VertexIndex)
		{
			FUnLiveVector4 Position = FUnLiveVector4(VertexArray[VertexIndex * 2], VertexArray[VertexIndex * 2 + 1], 0, 1);
			FULVector2f NewPos = Transform.TransformPoint(FULVector2f(Position.X, Position.Y) * SetupScale * FULVector2f(1.f, -1.f) + (WidgetSize / 2));

			FULVector2f MaskUV = FULVector2f(1.f, 1.f);
			if (bMask)
			{
				FUnLiveVector4 ClipPosition = MartixForDraw.TransformFVector4(Position);
				MaskUV = FULVector2f(ClipPosition.X, 1 + ClipPosition.Y);
				MaskUV /= ClipPosition.W;
			}
			FSlateVertex* VertexIndexData = bCreateIndexData ? &InterlottingLive2DVertexData[VerticesIndex + VertexIndex] : &InterlottingLive2DVertexData.AddDefaulted_GetRef();
			VertexIndexData->SetPosition(NewPos);
			VertexIndexData->MaterialTexCoords = FULVector2f(Opacity); // TexCoord4
			VertexIndexData->PixelSize[0] = bMask ? 1.f : 0.f; // TexCoord3 X
			VertexIndexData->PixelSize[1] = bInvertedMesk ? 1.f : 0.f; // TexCoord3 Y
			VertexIndexData->Color = ClipColor;
			VertexIndexData->SetTexCoords(FUnLiveVector4(UVArray[VertexIndex].X, 1 - UVArray[VertexIndex].Y, MaskUV.X, MaskUV.Y)); // TexCoord0 and TexCoord1

		}
		VerticesIndex += NumVertext;
	}
}

SUnLive2DViewUI::FUnLive2DSlateMaterialBrush::FUnLive2DSlateMaterialBrush(UTexture* InTexture)
	: FSlateBrush(ESlateBrushDrawType::Image, TEXT("None"), FMargin(), ESlateBrushTileType::NoTile, ESlateBrushImageType::FullColor, FVector2D(64.f, 64.f), FLinearColor::White, InTexture)
	, UnLive2DRenderType(EUnLive2DRenderType::RenderTarget)
{
	FString BrushName = TEXT("UnLive2DBrush");
	BrushName.AppendInt(UnLive2DBrushNameId++);
	ResourceName = FName(BrushName);
}

SUnLive2DViewUI::FUnLive2DSlateMaterialBrush::FUnLive2DSlateMaterialBrush(UMaterialInterface* InMaterial, const FVector2D& InImageSize)
	: FSlateBrush(ESlateBrushDrawType::Image, TEXT("None"), FMargin(), ESlateBrushTileType::NoTile, ESlateBrushImageType::FullColor, InImageSize, FLinearColor::White, InMaterial)
	, UnLive2DRenderType(EUnLive2DRenderType::Mesh)
{
	FString BrushName = TEXT("UnLive2DBrush");
	BrushName.AppendInt(UnLive2DBrushNameId++);
	ResourceName = FName(BrushName);
}
