#include "ThumbnailRenderer/UnLive2DThumbnailRenderer.h"
#include "Styling/SlateBrush.h"
#include "Slate/WidgetRenderer.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "Slate/SUnLive2DViewUI.h"
#include "Draw/UnLive2DSepRenderer.h"
#include "UnLive2DSetting.h"

UUnLive2DThumbnailRenderer::UUnLive2DThumbnailRenderer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUnLive2DThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	UUnLive2D* UnLive2D = Cast<UUnLive2D>(Object);
	if (UnLive2D == nullptr || UnLive2D->IsPendingKill()) return;

	SourceUnLive2D = UnLive2D;
	if (WidgetRenderer == nullptr)
	{
		const bool bUseGammaCorrection = true;
		WidgetRenderer = new FWidgetRenderer(bUseGammaCorrection);
		check(WidgetRenderer);
	}

	UTexture2D* CheckerboardTexture = UThumbnailManager::Get().CheckerboardTexture;

	FSlateBrush CheckerboardBrush;
	CheckerboardBrush.SetResourceObject(CheckerboardTexture);
	CheckerboardBrush.ImageSize = FVector2D(CheckerboardTexture->GetSizeX(), CheckerboardTexture->GetSizeY());
	CheckerboardBrush.Tiling = ESlateBrushTileType::Both;

	TSharedRef<SWidget> ThumbnailWidget =
		SNew(SOverlay)

		// Checkerboard
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(&CheckerboardBrush)
		]

		+ SOverlay::Slot()
		[
			SAssignNew(MySlateWidget, SUnLive2DViewUI, SourceUnLive2D)
			.OnUpDataRender(BIND_UOBJECT_DELEGATE(FOnUpDataRender, SlateUpDataRender))
			.OnInitUnLive2DRender(BIND_UOBJECT_DELEGATE(FOnInitUnLive2DRender, InitUnLive2DRender))
		];


	const FVector2D DrawSize((float)Width, (float)Height);
	const float DeltaTime = 0.f;
	WidgetRenderer->DrawWidget(RenderTarget, ThumbnailWidget, DrawSize, DeltaTime);
}

void UUnLive2DThumbnailRenderer::BeginDestroy()
{
	if (WidgetRenderer != nullptr)
	{
		BeginCleanup(WidgetRenderer);
		WidgetRenderer = nullptr;
	}
	MySlateWidget.Reset();

	Super::BeginDestroy();
}

const UUnLive2D* UUnLive2DThumbnailRenderer::GetUnLive2D() const
{
	return SourceUnLive2D;
}

TSharedRef<class FUnLive2DRenderState> UUnLive2DThumbnailRenderer::InitUnLive2DRender()
{
	if (!UnLive2DRenderPtr.IsValid())
	{
		const UUnLive2DSetting* Setting = GetDefault<UUnLive2DSetting>();
		UnLive2DRenderPtr = MakeShared<FUnLive2DRenderState>(this, GEditor->GetWorld());
		UnLive2DRenderPtr->SetUnLive2DMaterial(0, Cast<UMaterialInterface>(Setting->DefaultUnLive2DNormalMaterial_UI.TryLoad()));
		UnLive2DRenderPtr->SetUnLive2DMaterial(1, Cast<UMaterialInterface>(Setting->DefaultUnLive2DAdditiveMaterial_UI.TryLoad()));
		UnLive2DRenderPtr->SetUnLive2DMaterial(2, Cast<UMaterialInterface>(Setting->DefaultUnLive2DMultiplyMaterial_UI.TryLoad()));
		UnLive2DRenderPtr->InitRender(SourceUnLive2D, MySlateWidget->UnLive2DRawModel);
	}
	else
	{
		UnLive2DRenderPtr->InitRender(SourceUnLive2D, MySlateWidget->UnLive2DRawModel);
	}

	return UnLive2DRenderPtr.ToSharedRef();
}

void UUnLive2DThumbnailRenderer::SlateUpDataRender(TWeakPtr<class FUnLive2DRawModel> InUnLive2DRawModel)
{
	if (UnLive2DRenderPtr.IsValid())
	{
		// 限幅掩码・缓冲前处理方式的情况
		UnLive2DRenderPtr->UpdateRenderBuffers(InUnLive2DRawModel);
	}
}

