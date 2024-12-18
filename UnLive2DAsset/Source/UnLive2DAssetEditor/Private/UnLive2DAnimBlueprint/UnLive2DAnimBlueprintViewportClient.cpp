#include "UnLive2DAnimBlueprintViewportClient.h"
#include "UnLive2DRendererComponent.h"
#include "Animation/UnLive2DAnimBlueprint.h"
#include "SUnLive2DAnimBlueprintEditorViewport.h"

FUnLive2DAnimBlueprintViewportClient::FUnLive2DAnimBlueprintViewportClient(UUnLive2DAnimBlueprint* InUnLive2DAnimBlueprintEdited)
	: FUnLive2DViewportClient(InUnLive2DAnimBlueprintEdited->TargetUnLive2D)
{
	UnLive2DAnimBlueprintEditedLastFrame = InUnLive2DAnimBlueprintEdited;

	SetRealtime(true);

	DrawHelper.bDrawGrid = false;

	EngineShowFlags.DisableAdvancedFeatures();
	EngineShowFlags.SetCompositeEditorPrimitives(true);
}


void FUnLive2DAnimBlueprintViewportClient::Tick(float DeltaSeconds)
{
	FUnLive2DViewportClient::Tick(DeltaSeconds);

	if (!GIntraFrameDebuggingGameThread)
	{
		OwnedPreviewScene.GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
	}
}

FBox FUnLive2DAnimBlueprintViewportClient::GetDesiredFocusBounds() const
{
	if (AnimatedRenderComponent.IsValid())
	{
		return AnimatedRenderComponent->CalcLocalBounds().GetBox();
	}

	return FUnLive2DViewportClient::GetDesiredFocusBounds();
}
