#include "Slate/SUnLive2DViewUI.h"
#include "UnLive2D.h"
#include "Draw/UnLive2DSepRenderer.h"
#include "Templates/SharedPointer.h"
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
	OnUpDataRender = InArgs._OnUpDataRender;
	InitUnLive2D();
}

void SUnLive2DViewUI::SetUnLive2D(const UUnLive2D* InUnLive2D)
{
	InitUnLive2D();
	
}

void SUnLive2DViewUI::ReleaseRenderStateData()
{
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

	if (OwnerWidget->SourceUnLive2D == nullptr) return;

	if (!UnLive2DRawModel.IsValid())
	{
		UnLive2DRawModel = OwnerWidget->SourceUnLive2D->CreateLive2DRawModel();
		//UnLive2DRawModel->OnMotionPlayEnd.BindUObject(this, &SUnLive2DViewUI::OnMotionPlayeEnd);
	}
	else
	{
		if (UnLive2DRawModel->GetOwnerLive2D().IsValid() && UnLive2DRawModel->GetOwnerLive2D().Get() != OwnerWidget->SourceUnLive2D)
		{
			UnLive2DRawModel.Reset();
			UnLive2DRawModel = OwnerWidget->SourceUnLive2D->CreateLive2DRawModel();
			//UnLive2DRawModel->OnMotionPlayEnd.BindUObject(this, &SUnLive2DViewUI::OnMotionPlayeEnd);
		}
	}

	if (OwnerWidget.IsValid())
	{
		OwnerWidget->InitUnLive2DRender();
	}

}

void SUnLive2DViewUI::UpDateMesh(const FGeometry& AllottedGeometry, int32 DrawableIndex, class CubismClippingContext* ClipContext, const FWidgetStyle& InWidgetStyle)
{
	//TArray<SlateIndex> Live2DIndexData;

	const Csm::CubismModel* UnLive2DModel = UnLive2DRawModel->GetModel();

	FLinearColor WidgetStyleColor = InWidgetStyle.GetColorAndOpacityTint();

	csmFloat32 Opacity = UnLive2DModel->GetDrawableOpacity(DrawableIndex) * FMath::Clamp(WidgetStyleColor.A, 0.f, 1.f) ; // 获取不透明度

	if (Opacity == 0.f) return;

	UMaterialInstanceDynamic* DynamicMat = OwnerWidget->UnLive2DRenderPtr->GetMaterialInstanceDynamicToIndex(UnLive2DModel, DrawableIndex, ClipContext != nullptr); // 获取当前动态材质

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

	const csmInt32 NumVertext = UnLive2DModel->GetDrawableVertexCount(DrawableIndex); // 获得Drawable顶点的个数

	const csmFloat32* UVArray = reinterpret_cast<csmFloat32*>(const_cast<Live2D::Cubism::Core::csmVector2*>(UnLive2DModel->GetDrawableVertexUvs(DrawableIndex))); // 获取UV组
	const csmFloat32* VertexArray = const_cast<csmFloat32*>(UnLive2DModel->GetDrawableVertices(DrawableIndex)); // 顶点组

	FUnLiveVector4 ChanelFlag;
	FUnLiveMatrix MartixForDraw = OwnerWidget->UnLive2DRenderPtr->GetUnLive2DPosToClipMartix(ClipContext, ChanelFlag);

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
		FVector2D UV = FVector2D(UVArray[VertexIndex * 2], 1 - UVArray[VertexIndex * 2 + 1]); // UE UV坐标与Live2D的Y坐标是相反的
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

			//VertexIndexData->Color = FColor(255 * ChanelFlag.X, 255 * ChanelFlag.Y, 255 * ChanelFlag.Z, 255 * ChanelFlag.W);
			VertexIndexData->PixelSize[0] = ChanelFlag.Z;
			VertexIndexData->PixelSize[1] = ChanelFlag.W;

#if UE_VERSION_OLDER_THAN(5,0,0)
			VertexIndexData->MaterialTexCoords = MaskUV;
			VertexIndexData->SetTexCoords(FVector4(UV.X, UV.Y, ChanelFlag.X, ChanelFlag.Y));
#else
			VertexIndexData->MaterialTexCoords = FVector2f(MaskUV.X, MaskUV.Y);
			VertexIndexData->SetTexCoords(FVector4f(UV.X, UV.Y, ChanelFlag.X, ChanelFlag.Y));
#endif

		}
		else
		{
#if UE_VERSION_OLDER_THAN(5,0,0)
			VertexIndexData->MaterialTexCoords = UV;
			VertexIndexData->SetTexCoords(FVector4(UV.X, UV.Y, 1, 1));
#else
			VertexIndexData->MaterialTexCoords = FVector2f(UV.X, UV.Y);
			VertexIndexData->SetTexCoords(FVector4f(UV.X, UV.Y, 1, 1));
#endif
			//VertexIndexData->Color = FColor(255, 255, 255, 255);
		}

		
		FVector2D NewPos = Transform.TransformPoint(FVector2D(Position.X, Position.Y) * SetupScale * FVector2D(1.f, -1.f) + (WidgetSize / 2));

#if UE_VERSION_OLDER_THAN(5,0,0)
		//VertexIndexData->SetTexCoords(FVector4(UV.X, UV.Y, Opacity, Opacity));
		VertexIndexData->SetPosition(NewPos);
#else
		//VertexIndexData->SetTexCoords(FVector4f(UV.X, UV.Y, Opacity, Opacity));
		VertexIndexData->SetPosition(FVector2f(NewPos.X, NewPos.Y));
#endif
		VertexIndexData->Color = FColor(255 * WidgetStyleColor.R, 255 * WidgetStyleColor.G, 255 * WidgetStyleColor.B, 255 * Opacity);
	}

	const csmInt32 VertexIndexCount = UnLive2DModel->GetDrawableVertexIndexCount(DrawableIndex); // 获得Drawable的顶点索引个数

	check(VertexIndexCount > 0 && "Bad Index Count");

	const csmUint16* IndicesArray = const_cast<csmUint16*>(UnLive2DModel->GetDrawableVertexIndices(DrawableIndex)); //顶点索引

	for (int32 VertexIndex = 0; VertexIndex < VertexIndexCount; ++VertexIndex)
	{
		MeshSectionData->InterlottingLive2DIndexData.Add(InterlottingIndiceIndex + (SlateIndex)IndicesArray[VertexIndex]);
	}
	
}

void SUnLive2DViewUI::Flush(int32 LayerId, FSlateWindowElementList& OutDrawElements, bool bParentEnabled)
{

	const bool bIsEnabled = ShouldBeEnabled(bParentEnabled);

	const ESlateDrawEffect DrawEffects = bIsEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
	for (int32 i = 0; i < UnLive2DCustomVertsData.Num(); i++)
	{
		TSharedPtr<FSlateBrush> Brush = MakeShareable(new FUnLive2DSlateMaterialBrush(UnLive2DCustomVertsData[i].InterlottingDynamicMat, FVector2D(64.f, 64.f)));
		FSlateResourceHandle RenderingResourceHandle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*Brush);

		FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, RenderingResourceHandle, UnLive2DCustomVertsData[i].InterlottingLive2DVertexData, UnLive2DCustomVertsData[i].InterlottingLive2DIndexData, nullptr, 0, 0, DrawEffects);
	}

	UnLive2DCustomVertsData.Empty();
}

void SUnLive2DViewUI::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SLeafWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	if (!UnLive2DRawModel.IsValid() || OwnerWidget->SourceUnLive2D == nullptr) return;

	UnLive2DRawModel->OnUpDate(InDeltaTime * OwnerWidget->SourceUnLive2D->PlayRate);
}

int32 SUnLive2DViewUI::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (!UnLive2DRawModel.IsValid()) return LayerId;

	if (OnUpDataRender.IsBound())
	{
		OnUpDataRender.Execute(UnLive2DRawModel);
	}

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

	SUnLive2DViewUI* ThisPtr = const_cast<SUnLive2DViewUI*>(this);
	// 合批
	for (csmInt32 i = 0; i < DrawableCount; i++)
	{
		const csmInt32 DrawableIndex = SortedDrawableIndexList[i];
		// <Drawable如果不是显示状态，则通过处理
		if (!UnLive2DModel->GetDrawableDynamicFlagIsVisible(DrawableIndex)) continue;

		if (0 == UnLive2DModel->GetDrawableVertexIndexCount(DrawableIndex)) continue;

		CubismClippingContext* ClipContext = OwnerWidget->UnLive2DRenderPtr->GetClipContextInDrawableIndex(DrawableIndex);

		const bool IsMaskDraw = (nullptr != ClipContext);

		ThisPtr->UpDateMesh(AllottedGeometry, DrawableIndex, ClipContext, InWidgetStyle);
	}

	// 描画
	ThisPtr->Flush(LayerId, OutDrawElements, bParentEnabled);

	return LayerId;
}

FVector2D SUnLive2DViewUI::ComputeDesiredSize(float) const
{
	return FVector2D(64.f, 64.f);
}
