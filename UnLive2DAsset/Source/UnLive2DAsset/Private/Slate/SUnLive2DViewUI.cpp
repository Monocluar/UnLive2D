#include "Slate/SUnLive2DViewUI.h"
#include "UnLive2D.h"
#include "Draw/UnLive2DSepRenderer.h"
#include "Templates/SharedPointer.h"
#include "FWPort/UnLive2DRawModel.h"
#include "Model/CubismModel.hpp"
#include "Styling/SlateBrush.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UnLive2DViewRendererUI.h"
#include "Framework/Application/SlateApplication.h"

#if UE_VERSION_OLDER_THAN(5,0,0)
typedef FMatrix FUnLiveMatrix;
typedef FVector4 FUnLiveVector4;
#else
using namespace UE::Math;
typedef FMatrix44f FUnLiveMatrix;
typedef FVector4f FUnLiveVector4;
#endif

static int32 UnLive2DBrushNameId = 0;

struct FUnLive2DSlateMaterialBrush : public FSlateBrush
{
	FUnLive2DSlateMaterialBrush(UMaterialInterface* InMaterial, const FVector2D& InImageSize)
		: FSlateBrush(ESlateBrushDrawType::Image, TEXT("None"), FMargin(), ESlateBrushTileType::NoTile, ESlateBrushImageType::FullColor, InImageSize, FLinearColor::White, InMaterial)
	{
		FString BrushName = TEXT("UnLive2DBrush");
		BrushName.AppendInt(UnLive2DBrushNameId++);
		ResourceName = FName(BrushName);
	}
};

void SUnLive2DViewUI::Construct(const FArguments& InArgs, UUnLive2DViewRendererUI* InRendererUI)
{
	OwnerWidget = InRendererUI;

	InitUnLive2D();
}

void SUnLive2DViewUI::SetUnLive2D(const UUnLive2D* InUnLive2D)
{
	UnLive2DWeak = InUnLive2D;

	InitUnLive2D();
	
}

const UUnLive2D* SUnLive2DViewUI::GetUnLive2D() const
{
	return UnLive2DWeak.Get();
}

void SUnLive2DViewUI::ReleaseRenderStateData()
{
	UnLive2DRenderPtr.Reset();
	UnLive2DRawModel.Reset();
}

void SUnLive2DViewUI::PlayMotion(class UUnLive2DMotion* InMotion)
{
	if (UnLive2DRawModel.IsValid())
	{
		UnLive2DRawModel->StartMotion(InMotion);
	}
}

void SUnLive2DViewUI::PlayExpression(class UUnLive2DExpression* InExpression)
{
	if (UnLive2DRawModel.IsValid())
	{
		UnLive2DRawModel->StartExpressions(InExpression);
	}
}

void SUnLive2DViewUI::InitUnLive2D()
{
	if (!FSlateApplication::IsInitialized()) return;

	if (!UnLive2DWeak.IsValid()) return;

	if (!UnLive2DRawModel.IsValid())
	{
		UnLive2DRawModel = UnLive2DWeak->CreateLive2DRawModel();
		//UnLive2DRawModel->OnMotionPlayEnd.BindUObject(this, &SUnLive2DViewUI::OnMotionPlayeEnd);
	}
	else
	{
		if (UnLive2DRawModel->GetOwnerLive2D().IsValid() && UnLive2DRawModel->GetOwnerLive2D().Get() != UnLive2DWeak.Get())
		{
			UnLive2DRawModel.Reset();
			UnLive2DRawModel = UnLive2DWeak->CreateLive2DRawModel();
			//UnLive2DRawModel->OnMotionPlayEnd.BindUObject(this, &SUnLive2DViewUI::OnMotionPlayeEnd);
		}
	}

	if (!UnLive2DRenderPtr.IsValid())
	{
		UnLive2DRenderPtr = MakeShared<FUnLive2DRenderState>(SharedThis(this));
		UnLive2DRenderPtr->InitRender(UnLive2DWeak.Get(), UnLive2DRawModel);
	}
	else
	{
		UnLive2DRenderPtr->InitRender(UnLive2DWeak.Get(), UnLive2DRawModel);
	}
}

void SUnLive2DViewUI::UpDateMesh(const FGeometry& AllottedGeometry, int32 DrawableIndex, class CubismClippingContext* ClipContext)
{
	//TArray<SlateIndex> Live2DIndexData;

	const Csm::CubismModel* UnLive2DModel = UnLive2DRawModel->GetModel();

	csmFloat32 Opacity = UnLive2DModel->GetDrawableOpacity(DrawableIndex); // ??????????????????

	UMaterialInstanceDynamic* DynamicMat = UnLive2DRenderPtr->GetMaterialInstanceDynamicToIndex(UnLive2DModel, DrawableIndex, ClipContext != nullptr); // ????????????????????????

	if (!IsValid(DynamicMat)) return;

	FVector2D BoundsSize = FVector2D(UnLive2DModel->GetCanvasWidth(), UnLive2DModel->GetCanvasHeight());

	const FVector2D WidgetSize = AllottedGeometry.GetLocalSize();

	const FVector2D WidgetScale = WidgetSize / BoundsSize;
	const float SetupScale = WidgetScale.GetMin();
	const FSlateRenderTransform &Transform = AllottedGeometry.GetAccumulatedRenderTransform();

	FCustomVertsData* MeshSectionData = nullptr;

	if (UnLive2DCustomVertsData.Num() == 0 || UnLive2DCustomVertsData.Top().InterlottingDynamicMat != DynamicMat)
	{
		MeshSectionData = &UnLive2DCustomVertsData.AddDefaulted_GetRef();
		MeshSectionData->InterlottingDynamicMat = DynamicMat;
	}
	else
	{
		MeshSectionData = &UnLive2DCustomVertsData.Top();
	}


	int32 InterlottingIndiceIndex = MeshSectionData->InterlottingLive2DVertexData.Num();

	const csmInt32 NumVertext = UnLive2DModel->GetDrawableVertexCount(DrawableIndex); // ??????Drawable???????????????

	const csmFloat32* UVArray = reinterpret_cast<csmFloat32*>(const_cast<Live2D::Cubism::Core::csmVector2*>(UnLive2DModel->GetDrawableVertexUvs(DrawableIndex))); // ??????UV???
	const csmFloat32* VertexArray = const_cast<csmFloat32*>(UnLive2DModel->GetDrawableVertices(DrawableIndex)); // ?????????

	FUnLiveVector4 ChanelFlag;
	FUnLiveMatrix MartixForDraw = UnLive2DRenderPtr->GetUnLive2DPosToClipMartix(ClipContext, ChanelFlag);

	/*Live2DVertexData.SetNumUninitialized(NumVertext);
	FSlateVertex* VertexDataPtr = (FSlateVertex*)Live2DVertexData.GetData();*/

	for (int32 VertexIndex = 0; VertexIndex < NumVertext; ++VertexIndex)
	{

#if UE_VERSION_OLDER_THAN(5,0,0)
		FVector4 Position = FVector4(VertexArray[VertexIndex * 2], VertexArray[VertexIndex * 2 + 1], 0, 1);
#else
		TVector4<float> Position = TVector4<float>(VertexArray[VertexIndex * 2], VertexArray[VertexIndex * 2 + 1], 0, 1);
#endif

		FSlateVertex* VertexIndexData = &MeshSectionData->InterlottingLive2DVertexData.AddDefaulted_GetRef();

		float MaskVal = 1;
		FVector2D UV = FVector2D(UVArray[VertexIndex * 2], 1 - UVArray[VertexIndex * 2 + 1]); // UE UV?????????Live2D???Y??????????????????
		if (ClipContext != nullptr)
		{
#if UE_VERSION_OLDER_THAN(5,0,0)
			FVector4 ClipPosition;
#else
			TVector4<float> ClipPosition;
#endif

			ClipPosition = MartixForDraw.TransformFVector4(Position);
			FVector2D MaskUV = FVector2D(ClipPosition.X, 1 + ClipPosition.Y);
			MaskUV /= ClipPosition.W;

			VertexIndexData->Color = FColor(255 * ChanelFlag.X, 255 * ChanelFlag.Y, 255 * ChanelFlag.Z, Opacity * 255);

#if UE_VERSION_OLDER_THAN(5,0,0)
			VertexIndexData->SetTexCoords(FVector4(UV, MaskUV));
#else
			VertexIndexData->SetTexCoords(FVector4f(UV.X, UV.Y, MaskUV.X, MaskUV.Y));
#endif

		}
		else
		{
#if UE_VERSION_OLDER_THAN(5,0,0)
			VertexIndexData->SetTexCoords(FVector4(UV, UV));
#else
			VertexIndexData->SetTexCoords(FVector4f(UV.X, UV.Y, UV.X, UV.Y));
#endif
			VertexIndexData->Color = FColor(255, 255, 255, Opacity * 255);
		}

		
		FVector2D NewPos = Transform.TransformPoint(FVector2D(Position.X, Position.Y) * SetupScale * FVector2D(1.f, -1.f) + (WidgetSize / 2));

#if UE_VERSION_OLDER_THAN(5,0,0)
		VertexIndexData->SetPosition(NewPos);
#else
		VertexIndexData->SetPosition(FVector2f(NewPos.X, NewPos.Y));
#endif

#if UE_VERSION_OLDER_THAN(5,0,0)
		VertexIndexData->MaterialTexCoords = UV;
#else
		VertexIndexData->MaterialTexCoords = FVector2f(UV.X, UV.Y);
#endif
		VertexIndexData->PixelSize[0] = 1;
		VertexIndexData->PixelSize[1] = 1;
	}

	const csmInt32 VertexIndexCount = UnLive2DModel->GetDrawableVertexIndexCount(DrawableIndex); // ??????Drawable?????????????????????

	check(VertexIndexCount > 0 && "Bad Index Count");

	const csmUint16* IndicesArray = const_cast<csmUint16*>(UnLive2DModel->GetDrawableVertexIndices(DrawableIndex)); //????????????

	for (int32 VertexIndex = 0; VertexIndex < VertexIndexCount; ++VertexIndex)
	{
		SlateIndex& Live2DIndexData = MeshSectionData->InterlottingLive2DIndexData.AddDefaulted_GetRef();
		Live2DIndexData =  InterlottingIndiceIndex + (SlateIndex)IndicesArray[VertexIndex];
	}
	
}

void SUnLive2DViewUI::Flush(int32 LayerId, FSlateWindowElementList& OutDrawElements)
{
	for (int32 i = 0; i < UnLive2DCustomVertsData.Num(); i++)
	{
		TSharedPtr<FSlateBrush> Brush = MakeShareable(new FUnLive2DSlateMaterialBrush(UnLive2DCustomVertsData[i].InterlottingDynamicMat, FVector2D(64.f, 64.f)));
		FSlateResourceHandle RenderingResourceHandle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*Brush);

		FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, RenderingResourceHandle, UnLive2DCustomVertsData[i].InterlottingLive2DVertexData, UnLive2DCustomVertsData[i].InterlottingLive2DIndexData, nullptr, 0, 0);
	}

	UnLive2DCustomVertsData.Empty();
}

void SUnLive2DViewUI::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SLeafWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	if (!UnLive2DWeak.IsValid() || !UnLive2DRawModel.IsValid()) return;

	UnLive2DRawModel->OnUpDate(InDeltaTime * UnLive2DWeak->PlayRate);
}

int32 SUnLive2DViewUI::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (UnLive2DRenderPtr.IsValid() && UnLive2DWeak.IsValid())
	{

		SUnLive2DViewUI* ThisPtr = const_cast<SUnLive2DViewUI*>(this);

		// ?????????????????????????????????????????????
		UnLive2DRenderPtr->UpdateRenderBuffers(UnLive2DRawModel);

		if (!UnLive2DRawModel.IsValid()) return LayerId;

		const Csm::CubismModel* UnLive2DModel = UnLive2DRawModel->GetModel();

		const Csm::csmInt32 DrawableCount = UnLive2DModel->GetDrawableCount();
		const Csm::csmInt32* RenderOrder = UnLive2DModel->GetDrawableRenderOrders();

		TArray<Csm::csmInt32> SortedDrawableIndexList;
		SortedDrawableIndexList.SetNum(DrawableCount);

		for (csmInt32 i = 0; i < DrawableCount; i++)
		{
			const csmInt32 Order = RenderOrder[i];

			SortedDrawableIndexList[Order] = i;
		}

		// ??????
		for (csmInt32 i = 0; i < DrawableCount; i++)
		{
			const csmInt32 DrawableIndex = SortedDrawableIndexList[i];
			// <Drawable??????????????????????????????????????????
			if (!UnLive2DModel->GetDrawableDynamicFlagIsVisible(DrawableIndex)) continue;

			if (0 == UnLive2DModel->GetDrawableVertexIndexCount(DrawableIndex)) continue;

			CubismClippingContext* ClipContext = UnLive2DRenderPtr->GetClipContextInDrawableIndex(DrawableIndex);

			const bool IsMaskDraw = (nullptr != ClipContext);

			ThisPtr->UpDateMesh(AllottedGeometry, DrawableIndex, ClipContext);
		}

		// ??????
		ThisPtr->Flush(LayerId, OutDrawElements);
	}


	return LayerId;
}

FVector2D SUnLive2DViewUI::ComputeDesiredSize(float) const
{
	return FVector2D(64.f, 64.f);
}
