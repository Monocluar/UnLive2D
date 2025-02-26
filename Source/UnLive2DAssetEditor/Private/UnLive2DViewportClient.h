#pragma once

#include "CoreMinimal.h"
#include "EditorViewportClient.h"
#include "SEditorViewport.h"


class UUnLive2D;
class UUnLive2DRendererComponent;

struct FViewportSelectionRectangle
{
	FVector2D TopLeft;
	FVector2D Dimensions;
	FLinearColor Color;
};

class FUnLive2DViewportClient : public FEditorViewportClient
{
public:
	/** Constructor */
	explicit FUnLive2DViewportClient(UUnLive2D* InUnLive2DBeingEdited, const TWeakPtr<class SEditorViewport>& InEditorViewportWidget = nullptr);
	~FUnLive2DViewportClient();

public:
	// FViewportClient interface
	virtual void Tick(float DeltaSeconds) override;
	// End of FViewportClient interface

	// FEditorViewportClient interface
	virtual FLinearColor GetBackgroundColor() const override;
	// End of FEditorViewportClient interface

	// FSerializableObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	// End of FSerializableObject interface

	// Called to request a focus on the current selection
	virtual void RequestFocusOnSelection(bool bInstant);

	/** Modifies the checkerboard texture's data */
	void ModifyCheckerboardTextureColors();

	void SetZoomPos(FVector2D InNewPos, float InNewZoom)
	{
		ZoomPos = InNewPos;
		ZoomAmount = InNewZoom;
	}

	// List of selection rectangles to draw
	TArray<FViewportSelectionRectangle> SelectionRectangles;

	FORCEINLINE TWeakObjectPtr<UUnLive2DRendererComponent> GetUnLive2DRenderComponent() const { return AnimatedRenderComponent; };
	
private:
	/** Initialize the checkerboard texture for the texture preview, if necessary */
	void SetupCheckerboardTexture(const FColor& ColorOne, const FColor& ColorTwo, int32 CheckerSize);

	/** Destroy the checkerboard texture if one exists */
	void DestroyCheckerboardTexture();

protected:
	void DrawSelectionRectangles(FViewport* InViewport, FCanvas* Canvas);

	virtual FBox GetDesiredFocusBounds() const
	{
		return FBox(ForceInitToZero);
	}

	virtual void DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas) override;
	virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;

protected:
	/** Checkerboard texture */
	UTexture2D* CheckerboardTexture;
	FVector2D ZoomPos;
	float ZoomAmount;

protected:

	TWeakObjectPtr<UUnLive2D> UnLive2DBeingEditedLastFrame;

	TWeakObjectPtr<UUnLive2DRendererComponent> AnimatedRenderComponent;

	// The preview scene
	FPreviewScene OwnedPreviewScene;
private:
	// Should we zoom to the focus bounds next tick?
	bool bDeferZoomToUnLive2D;
	bool bDeferZoomToUnLive2DIsInstant;
};