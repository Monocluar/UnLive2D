#include "UnLive2DViewViewportClient.h"
#include "UnLive2D.h"
#include "Utils.h"
#include "CanvasItem.h"
#include "HitProxies.h"

#include "UnLive2DAssetEditor.h"
#include "DrawDebugHelpers.h"
#include "UnLive2DRendererComponent.h"
#include "CanvasTypes.h"


FUnLive2DViewViewportClient::FUnLive2DViewViewportClient(TWeakObjectPtr<UUnLive2D> InUnLive2DBeingEdited)
	: FUnLive2DViewportClient(InUnLive2DBeingEdited)
{
	SetRealtime(true);

	bShowPivot = true;
	bShowSockets = true;
	bIsMousePressed = false;

	DrawHelper.bDrawGrid  = false;

	OldMousePoint = FIntPoint::ZeroValue;

	EngineShowFlags.DisableAdvancedFeatures();
	EngineShowFlags.SetCompositeEditorPrimitives(true);

}


void FUnLive2DViewViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	FUnLive2DViewportClient::Draw(View, PDI);

	/*if (bShowPivot && AnimatedRenderComponent.IsValid())
	{
		const bool bCanSelectPivot = false;
		const bool bHitTestingForPivot = PDI->IsHitTesting() && bCanSelectPivot;
		FUnrealEdUtils::DrawWidget(View, PDI, AnimatedRenderComponent->GetComponentTransform().ToMatrixWithScale(), 0, 0, EAxisList::YZ, EWidgetMovementMode::WMM_Translate, bHitTestingForPivot);
	}*/
	
}

void FUnLive2DViewViewportClient::DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas)
{

	if (!UnLive2DBeingEditedLastFrame.IsValid()) return;


	FUnLive2DViewportClient::DrawCanvas(InViewport, View, Canvas);
}

void FUnLive2DViewViewportClient::Tick(float DeltaSeconds)
{
	FUnLive2DViewportClient::Tick(DeltaSeconds);

	if (!GIntraFrameDebuggingGameThread)
	{
		OwnedPreviewScene.GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
	}

	if (bIsMousePressed)
	{
		FIntPoint MousePos;
		Viewport->GetMousePos(MousePos);

		if ((MousePos - OldMousePoint).Size() > 2)
		{
			EventOnLeftMouseMove(MousePos - OldMousePoint);
			MousePos = OldMousePoint;
		}

	}
}

void FUnLive2DViewViewportClient::TrackingStarted(const struct FInputEventState& InInputState, bool bIsDragging, bool bNudge)
{
	if (InInputState.GetInputEvent() == IE_Pressed && InInputState.GetKey() == EKeys::LeftMouseButton)
	{
		bIsMousePressed = true;
		FIntPoint MousePos;
		Viewport->GetMousePos(MousePos);
		EventOnLeftMouseDown(MousePos);
		OldMousePoint = MousePos;
	}

	FUnLive2DViewportClient::TrackingStarted(InInputState, bIsDragging, bNudge);
}

void FUnLive2DViewViewportClient::TrackingStopped()
{
	if (bIsMousePressed)
	{
		bIsMousePressed = false;
		EventOnLeftMouseUp(OldMousePoint);
	}
	FUnLive2DViewportClient::TrackingStopped();
}

void FUnLive2DViewViewportClient::DrawSocketNames(UPrimitiveComponent* PreviewComponent, FViewport& InViewport, FSceneView& View, FCanvas& Canvas)
{
	if (PreviewComponent == nullptr) return;

	const int32 HalfX = InViewport.GetSizeXY().X / 2;
	const int32 HalfY = InViewport.GetSizeXY().Y / 2;

	const FColor UnselectedSocketNameColor(255, 196, 196);
	const FColor SelectedSocketNameColor(FColor::White);

	TArray<FComponentSocketDescription> SocketList;
	PreviewComponent->QuerySupportedSockets(/*out*/ SocketList);

	for (const FComponentSocketDescription& Socket : SocketList)
	{
		const FVector SocketWorldPos = PreviewComponent->GetSocketLocation(Socket.Name);

		const FPlane Proj = View.Project(SocketWorldPos);
		if (Proj.W > 0.f)
		{
			const int32 XPos = HalfX + (HalfX * Proj.X);
			const int32 YPos = HalfY + (HalfY * (-Proj.Y));

			const bool bIsSelected = true;
			const FColor& SocketColor = bIsSelected ? SelectedSocketNameColor : UnselectedSocketNameColor;

			FCanvasTextItem Msg(FVector2D(XPos, YPos), FText::FromString(Socket.Name.ToString()), GEngine->GetMediumFont(), SocketColor);
			Msg.EnableShadow(FLinearColor::Black);
			Canvas.DrawItem(Msg);

			// 				//@TODO: Draws the current value of the rotation (probably want to keep this!)
			// 				if (bManipulating && StaticMeshEditorPtr.Pin()->GetSelectedSocket() == Socket)
			// 				{
			// 					// Figure out the text height
			// 					FTextSizingParameters Parameters(GEngine->GetSmallFont(), 1.0f, 1.0f);
			// 					UCanvas::CanvasStringSize(Parameters, *Socket->SocketName.ToString());
			// 					int32 YL = FMath::TruncToInt(Parameters.DrawYL);
			// 
			// 					DrawAngles(&Canvas, XPos, YPos + YL, 
			// 						Widget->GetCurrentAxis(), 
			// 						GetWidgetMode(),
			// 						Socket->RelativeRotation,
			// 						Socket->RelativeLocation);
			// 				}
		}
	}
}

FBox FUnLive2DViewViewportClient::GetDesiredFocusBounds() const
{
	if (!AnimatedRenderComponent.IsValid())
	{
		return FUnLive2DViewportClient::GetDesiredFocusBounds();
	}
	/*FVector2D Size = UnLive2DBeingEditedLastFrame->DrawSize;

	FVector2D Origin = Size  * UnLive2DBeingEditedLastFrame->Pivot; //原点坐标

	FVector2D MinPos = - Origin;

	FVector2D MaxPos = Size - Origin;

	return FBox(FVector(0, MinPos.X, MinPos.Y), FVector(0, MaxPos.X, MaxPos.Y));*/

	return FBox(AnimatedRenderComponent->CalcLocalBounds().GetBox().Min / 2.f, AnimatedRenderComponent->CalcLocalBounds().GetBox().Max / 2.f);
}

void FUnLive2DViewViewportClient::EventOnLeftMouseDown(FIntPoint MousePoint)
{
	/*if (Viewport == nullptr) return;

	FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(Viewport, GetScene(), EngineShowFlags));
	FSceneView* View = CalcSceneView(&ViewFamily);

	FVector StartPos = View->PixelToWorld(MousePoint.X, MousePoint.Y, 0);

	FVector2D TextureSpaceStartPos = AnimatedRenderComponent->ConvertWorldSpaceToTextureSpace(StartPos);

	const FIntPoint SourceTextureSize(UnLive2DBeingEditedLastFrame->DrawSize);
	const int32 SourceTextureWidth = SourceTextureSize.X;
	const int32 SourceTextureHeight = SourceTextureSize.Y;

	OldMouseTapPos.X = FMath::Clamp<int32>((int32)TextureSpaceStartPos.X, 0, SourceTextureWidth - 1);
	OldMouseTapPos.Y = FMath::Clamp<int32>((int32)TextureSpaceStartPos.Y, 0, SourceTextureHeight - 1);

	UnLive2DBeingEditedLastFrame->OnTap(OldMouseTapPos);*/

}

void FUnLive2DViewViewportClient::EventOnLeftMouseUp(FIntPoint MousePoint)
{

}

void FUnLive2DViewViewportClient::EventOnLeftMouseMove(FIntPoint MousePointDifference)
{
	OldMouseTapPos += MousePointDifference;

	//UnLive2DBeingEditedLastFrame->OnDrag(OldMouseTapPos);
	UE_LOG(LogUnLive2DEditor, Log, TEXT("MousePos:X:%d,Y:%d"), MousePointDifference.X, MousePointDifference.Y);
}