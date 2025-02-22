#include "UnLive2DViewportClient.h"
#include "RenderUtils.h"
#include "AssetEditorModeManager.h"
#include "ImageUtils.h"
#include "CanvasTypes.h"
#include "Misc/EngineVersionComparison.h"
#include "UnLive2DRendererComponent.h"
#include "UnLive2D.h"
#include "MouseDeltaTracker.h"

/*
const FVector UnLive2DAxisX = FVector(1.0f, 0.0f, 0.0f);
const FVector UnLive2DAxisY = FVector(0.0f, 0.0f, 1.0f);
FVector UnLive2DAxisX(1.0f, 0.0f, 0.0f);
FVector UnLive2DAxisY(0.0f, 0.0f, 1.0f);
FVector UnLive2DAxisZ(0.0f, 1.0f, 0.0f);*/

FUnLive2DViewportClient::FUnLive2DViewportClient(UUnLive2D* InUnLive2DBeingEdited, const TWeakPtr<class SEditorViewport>& InEditorViewportWidget /*= nullptr*/)
	: FEditorViewportClient(nullptr, nullptr, InEditorViewportWidget)
	, CheckerboardTexture(nullptr)
{
	//bOwnsModeTools = true;
	ZoomPos = FVector2D::ZeroVector;
	ZoomAmount = 1.0f;

	UnLive2DBeingEditedLastFrame = InUnLive2DBeingEdited;
	PreviewScene = &OwnedPreviewScene;

	SetViewModes(VMI_Lit, VMI_Lit);
	SetViewportType(LVT_OrthoNegativeYZ);
	bDeferZoomToUnLive2D = true;
	bDeferZoomToUnLive2DIsInstant = true;

	SetInitialViewTransform(LVT_Perspective, FVector(0.0f, 0.0f, 0.0f), FVector(0.0f, 0.0f, 0.0f).Rotation(), 1.f);

	AnimatedRenderComponent = NewObject<UUnLive2DRendererComponent>();
	AnimatedRenderComponent->SetUnLive2D(UnLive2DBeingEditedLastFrame.Get());
	PreviewScene->AddComponent(AnimatedRenderComponent.Get(), FTransform::Identity);

	//SetShowGrid();

}

FUnLive2DViewportClient::~FUnLive2DViewportClient()
{
	AnimatedRenderComponent->DestroyComponent();
}

void FUnLive2DViewportClient::Tick(float DeltaSeconds)
{
	FIntPoint Size = Viewport->GetSizeXY();
	if (bDeferZoomToUnLive2D && (Size.X > 0) && (Size.Y > 0))
	{
		FBox BoundsToFocus = GetDesiredFocusBounds();
		/*if (ViewportType != LVT_Perspective)
		{
			TGuardValue<ELevelViewportType> SaveViewportType(ViewportType, LVT_Perspective);
			FocusViewportOnBox(BoundsToFocus, bDeferZoomToUnLive2DIsInstant);
		}*/

		FocusViewportOnBox(BoundsToFocus, bDeferZoomToUnLive2DIsInstant);
		bDeferZoomToUnLive2D = false;
	}

	FEditorViewportClient::Tick(DeltaSeconds);
}

FLinearColor FUnLive2DViewportClient::GetBackgroundColor() const
{
	//return FLinearColor(55, 55, 55);
	return FEditorViewportClient::GetBackgroundColor();
}

void FUnLive2DViewportClient::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEditorViewportClient::AddReferencedObjects(Collector);

	Collector.AddReferencedObject(CheckerboardTexture);
}

void FUnLive2DViewportClient::RequestFocusOnSelection(bool bInstant)
{
	bDeferZoomToUnLive2D = true;
	bDeferZoomToUnLive2DIsInstant = bInstant;
}

void FUnLive2DViewportClient::ModifyCheckerboardTextureColors()
{
	const FColor ColorOne = FColor(128, 128, 128);//TextureEditorPtr.Pin()->GetCheckeredBackground_ColorOne();
	const FColor ColorTwo = FColor(64, 64, 64);//TextureEditorPtr.Pin()->GetCheckeredBackground_ColorTwo();
	const int32 CheckerSize = 32;//TextureEditorPtr.Pin()->GetCheckeredBackground_Size();

	DestroyCheckerboardTexture();
	SetupCheckerboardTexture(ColorOne, ColorTwo, CheckerSize);
}

void FUnLive2DViewportClient::SetupCheckerboardTexture(const FColor& ColorOne, const FColor& ColorTwo, int32 CheckerSize)
{
	if (CheckerboardTexture == nullptr)
	{
		CheckerboardTexture = FImageUtils::CreateCheckerboardTexture(ColorOne, ColorTwo, CheckerSize);
	}
}

void FUnLive2DViewportClient::DestroyCheckerboardTexture()
{
	if (CheckerboardTexture)
	{

#if ENGINE_MAJOR_VERSION < 5
	if (CheckerboardTexture->Resource)
#else
	if (CheckerboardTexture->GetResource())
#endif
	{
		CheckerboardTexture->ReleaseResource();
	}

#if ENGINE_MAJOR_VERSION < 5
		CheckerboardTexture->MarkPendingKill();
#else
		CheckerboardTexture->MarkAsGarbage();
#endif
		CheckerboardTexture = nullptr;
	}
}

void FUnLive2DViewportClient::DrawSelectionRectangles(FViewport* InViewport, FCanvas* Canvas)
{
	for (auto RectangleIt = SelectionRectangles.CreateConstIterator(); RectangleIt; ++RectangleIt)
	{
		const FViewportSelectionRectangle& Rect = *RectangleIt;

		const float X = (Rect.TopLeft.X - ZoomPos.X) * ZoomAmount;
		const float Y = (Rect.TopLeft.Y - ZoomPos.Y) * ZoomAmount;
		const float W = Rect.Dimensions.X * ZoomAmount;
		const float H = Rect.Dimensions.Y * ZoomAmount;
		const bool bAlphaBlend = true;

		Canvas->DrawTile(X, Y, W, H, 0, 0, 1, 1, Rect.Color, GWhiteTexture, bAlphaBlend);
	}
}

void FUnLive2DViewportClient::DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas)
{
	const bool bIsHitTesting = Canvas.IsHitTesting();
	if (!bIsHitTesting)
	{
		Canvas.SetHitProxy(nullptr);
	}
	FEditorViewportClient::DrawCanvas(InViewport, View, Canvas);
}

void FUnLive2DViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	if (UnLive2DBeingEditedLastFrame.IsValid())
	{
		for (UTexture2D* Item : UnLive2DBeingEditedLastFrame->TextureAssets)
		{
			if (Item)
			{
				Item->SetForceMipLevelsToBeResident(30.0f);
				Item->WaitForStreaming();
			}
		}
	}

	FEditorViewportClient::Draw(View, PDI);
}

