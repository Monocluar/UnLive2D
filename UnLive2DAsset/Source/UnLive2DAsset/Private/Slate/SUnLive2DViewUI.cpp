#include "Slate/SUnLive2DViewUI.h"
#include "UnLive2D.h"
#include "Draw/UnLive2DSepRenderer.h"
#include "Templates/SharedPointer.h"
#include "FWPort/UnLive2DRawModel.h"
#include "Model/CubismModel.hpp"
#include "Styling/SlateBrush.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UnLive2DViewRendererUI.h"

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

void SUnLive2DViewUI::Construct(const FArguments& InArgs)
{
	OwnerWidget = InArgs._OwnerWidget;

	InitUnLive2D();
}

SUnLive2DViewUI::~SUnLive2DViewUI()
{
	UnLive2DRenderPtr.Reset();
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

void SUnLive2DViewUI::InitUnLive2D()
{
	if (!FSlateApplication::IsInitialized()) return;

	if (!UnLive2DRenderPtr.IsValid() && UnLive2DWeak.IsValid())
	{
		UnLive2DRenderPtr = MakeShared<FUnLive2DRenderState>(SharedThis(this));
		UnLive2DRenderPtr->InitRender(UnLive2DWeak.Get());
	}
}

void SUnLive2DViewUI::UpDateMesh(int32 LayerId, FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 DrawableIndex, class CubismClippingContext* ClipContext, int32& ElementIndex)
{
	TArray<FVector2D> Live2DVertexs;
	TArray<FSlateVertex> Live2DVertexData;
	TArray<SlateIndex> Live2DIndexData;

	//TArray<FVector2D> Live2DUV1; // 遮罩缓冲的UV坐标
	//TArray<FVector2D> Live2DUV2; // ChannelFlag XY
	//TArray<FVector2D> Live2DUV3; // ChannelFlag ZW

	const Csm::CubismModel* UnLive2DModel = UnLive2DWeak->GetUnLive2DRawModel().Pin()->GetModel();

	csmFloat32 Opacity = UnLive2DModel->GetDrawableOpacity(DrawableIndex); // 获取不透明度

	UMaterialInstanceDynamic* DynamicMat = UnLive2DRenderPtr->GetMaterialInstanceDynamicToIndex(UnLive2DModel, DrawableIndex, ClipContext != nullptr); // 获取当前动态材质

	if (!IsValid(DynamicMat)) return;

	const csmInt32 NumVertext = UnLive2DModel->GetDrawableVertexCount(DrawableIndex); // 获得Drawable顶点的个数

	const csmFloat32* UVArray = reinterpret_cast<csmFloat32*>(const_cast<Live2D::Cubism::Core::csmVector2*>(UnLive2DModel->GetDrawableVertexUvs(DrawableIndex))); // 获取UV组
	const csmFloat32* VertexArray = const_cast<csmFloat32*>(UnLive2DModel->GetDrawableVertices(DrawableIndex)); // 顶点组

	FUnLiveVector4 ChanelFlag;
	FUnLiveMatrix MartixForDraw = UnLive2DRenderPtr->GetUnLive2DPosToClipMartix(ClipContext, ChanelFlag);

	Live2DVertexData.SetNumUninitialized(NumVertext);
	FSlateVertex* VertexDataPtr = (FSlateVertex*)Live2DVertexData.GetData();


	for (int32 VertexIndex = 0; VertexIndex < NumVertext; ++VertexIndex)
	{

#if UE_VERSION_OLDER_THAN(5,0,0)
		FVector4 Position = FVector4(VertexArray[VertexIndex * 2], VertexArray[VertexIndex * 2 + 1], 0, 1);
#else
		TVector4<float> Position = TVector4<float>(VertexArray[VertexIndex * 2], VertexArray[VertexIndex * 2 + 1], 0, 1);
#endif

		FSlateVertex* VertexIndexData = &VertexDataPtr[VertexIndex];

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

			//Live2DUV1.Add(MaskUV);
			//Live2DUV2.Add(FVector2D(ChanelFlag.X, ChanelFlag.Y));
			//Live2DUV3.Add(FVector2D(ChanelFlag.Z, ChanelFlag.W));
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
			VertexIndexData->SetTexCoords(FVector4(UV, UV)));
#else
			VertexIndexData->SetTexCoords(FVector4f(UV.X, UV.Y, UV.X, UV.Y));
#endif
			VertexIndexData->Color = FColor(255, 255, 255, Opacity * 255);
		}

		Live2DVertexs.Add(FVector2D(Position.X, Position.Y));

#if UE_VERSION_OLDER_THAN(5,0,0)
		VertexIndexData->MaterialTexCoords = UV;
#else
		VertexIndexData->MaterialTexCoords = FVector2f(UV.X, UV.Y);
#endif
		VertexIndexData->PixelSize[0] = 1;
		VertexIndexData->PixelSize[1] = 1;
	}

	FVector2D BoundsSize = FVector2D(UnLive2DModel->GetCanvasWidth(), UnLive2DModel->GetCanvasHeight());

	const csmInt32 VertexIndexCount = UnLive2DModel->GetDrawableVertexIndexCount(DrawableIndex); // 获得Drawable的顶点索引个数

	check(VertexIndexCount > 0 && "Bad Index Count");

	const csmUint16* IndicesArray = const_cast<csmUint16*>(UnLive2DModel->GetDrawableVertexIndices(DrawableIndex)); //顶点索引

	Live2DIndexData.SetNumUninitialized(VertexIndexCount);
	SlateIndex* IndexDataPtr = (SlateIndex* )Live2DIndexData.GetData();

	for (int32 VertexIndex = 0; VertexIndex < VertexIndexCount; ++VertexIndex)
	{
		IndexDataPtr[VertexIndex] = (SlateIndex)IndicesArray[VertexIndex];
	}

	const FVector2D WidgetSize = AllottedGeometry.GetLocalSize();

	const FVector2D WidgetScale = WidgetSize / BoundsSize;
	const float SetupScale = WidgetScale.GetMin();

	const FSlateRenderTransform &Transform = AllottedGeometry.GetAccumulatedRenderTransform();

	for (int32 i = 0; i < Live2DVertexData.Num(); i++)
	{
		FSlateVertex* VertexIndexData = &VertexDataPtr[i];

		FVector2D NewPos = Transform.TransformPoint(Live2DVertexs[i] * SetupScale * FVector2D(1.f, -1.f) + (WidgetSize / 2));

#if UE_VERSION_OLDER_THAN(5,0,0)
		VertexIndexData->SetPosition(NewPos);
#else
		VertexIndexData->SetPosition(FVector2f(NewPos.X, NewPos.Y));
#endif
	}

	if (IsValid(DynamicMat))
	{
		TSharedPtr<FSlateBrush> Brush = MakeShareable(new FUnLive2DSlateMaterialBrush(DynamicMat, FVector2D(64.f, 64.f)));
		FSlateResourceHandle RenderingResourceHandle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*Brush);

		FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, RenderingResourceHandle, Live2DVertexData, Live2DIndexData, nullptr, 0, 0);
	}

	ElementIndex++;
}

void SUnLive2DViewUI::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SLeafWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	if (!UnLive2DWeak.IsValid()) return;

	if (FUnLive2DRawModel* RawModel = UnLive2DWeak->GetUnLive2DRawModel().Pin().Get())
	{
		RawModel->OnUpDate(InDeltaTime * UnLive2DWeak->PlayRate);
	}
}

int32 SUnLive2DViewUI::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (UnLive2DRenderPtr.IsValid() && UnLive2DWeak.IsValid())
	{

		SUnLive2DViewUI* ThisPtr = const_cast<SUnLive2DViewUI*>(this);

		// 限幅掩码・缓冲前处理方式的情况
		UnLive2DRenderPtr->UpdateRenderBuffers();

		const Csm::CubismModel* UnLive2DModel = UnLive2DWeak->GetUnLive2DRawModel().Pin()->GetModel();

		const Csm::csmInt32 DrawableCount = UnLive2DModel->GetDrawableCount();
		const Csm::csmInt32* RenderOrder = UnLive2DModel->GetDrawableRenderOrders();

		TArray<Csm::csmInt32> SortedDrawableIndexList;
		SortedDrawableIndexList.SetNum(DrawableCount);

		for (csmInt32 i = 0; i < DrawableCount; i++)
		{
			const csmInt32 Order = RenderOrder[i];

			SortedDrawableIndexList[Order] = i;
		}

		int32 MeshSection = 0; // 根据顺序绘制

		// 描画
		for (csmInt32 i = 0; i < DrawableCount; i++)
		{
			const csmInt32 DrawableIndex = SortedDrawableIndexList[i];
			// <Drawable如果不是显示状态，则通过处理
			if (!UnLive2DModel->GetDrawableDynamicFlagIsVisible(DrawableIndex)) continue;

			if (0 == UnLive2DModel->GetDrawableVertexIndexCount(DrawableIndex)) continue;

			CubismClippingContext* ClipContext = UnLive2DRenderPtr->GetClipContextInDrawableIndex(DrawableIndex);

			const bool IsMaskDraw = (nullptr != ClipContext);

			ThisPtr->UpDateMesh(LayerId, OutDrawElements, AllottedGeometry, DrawableIndex, ClipContext, MeshSection);
		}

	}


	return LayerId;
}

FVector2D SUnLive2DViewUI::ComputeDesiredSize(float) const
{
	return FVector2D(64.f, 64.f);
}
