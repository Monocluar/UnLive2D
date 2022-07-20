#pragma once

#include "CoreMinimal.h"
#include "UnLive2DViewportClient.h"

class UUnLive2D;
class UUnLive2DRendererComponent;
class FCanvas;
class SUnLive2DEditorViewport;

class FUnLive2DViewViewportClient : public FUnLive2DViewportClient
{
public:
	/** Constructor */
	FUnLive2DViewViewportClient(TWeakObjectPtr<UUnLive2D> InUnLive2DBeingEdited);

protected:
	// FViewportClient interface
	virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual void DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas) override;
	virtual void Tick(float DeltaSeconds) override;
	// End of FViewportClient interface

protected:

	// FEditorViewportClient interface
	virtual void TrackingStarted(const struct FInputEventState& InInputState, bool bIsDragging, bool bNudge) override;
	virtual void TrackingStopped() override;
	// End of FEditorViewportClient interface

public:
	void ToggleShowPivot() { bShowPivot = !bShowPivot; Invalidate(); }
	bool IsShowPivotChecked() const { return bShowPivot; }

	void ToggleShowSockets() { bShowSockets = !bShowSockets; Invalidate(); }
	bool IsShowSocketsChecked() const { return bShowSockets; }

protected:

	void DrawSocketNames(UPrimitiveComponent* PreviewComponent, FViewport& Viewport, FSceneView& View, FCanvas& Canvas);

	void DrawSockets(UPrimitiveComponent* PreviewComponent, const FSceneView* View, FPrimitiveDrawInterface* PDI);

	virtual FBox GetDesiredFocusBounds() const override;

	void EventOnLeftMouseDown(FIntPoint MousePoint);
	void EventOnLeftMouseUp(FIntPoint MousePoint);
	void EventOnLeftMouseMove(FIntPoint MousePointDifference);


private:

	// Should we show the sprite pivot?
	bool bShowPivot;

	// Should we show sockets?
	bool bShowSockets;

	bool bIsMousePressed;

	FIntPoint OldMousePoint;

	FIntPoint OldMouseTapPos;
};