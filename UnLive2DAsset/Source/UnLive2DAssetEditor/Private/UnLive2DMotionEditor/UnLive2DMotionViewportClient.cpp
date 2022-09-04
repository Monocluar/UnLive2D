#include "UnLive2DMotionViewportClient.h"
#include "UnLive2DRendererComponent.h"
#include "Animation/UnLive2DAnimBase.h"
#include "SUnLive2DMotionEditorViewport.h"

FUnLive2DAnimBaseViewportClient::FUnLive2DAnimBaseViewportClient(TWeakObjectPtr<UUnLive2DAnimBase> InUnLive2DAnimBaseBegingEdited)
	: FUnLive2DViewportClient(InUnLive2DAnimBaseBegingEdited->UnLive2D)
{
	UnLive2DAnimBaseBeingEditedLastFrame = InUnLive2DAnimBaseBegingEdited;

	SetRealtime(true);

	DrawHelper.bDrawGrid = false;

	EngineShowFlags.DisableAdvancedFeatures();
	EngineShowFlags.SetCompositeEditorPrimitives(true);

	SetShowGrid();
}

void FUnLive2DAnimBaseViewportClient::Tick(float DeltaSeconds)
{
	FUnLive2DViewportClient::Tick(DeltaSeconds);

	if (!GIntraFrameDebuggingGameThread)
	{
		OwnedPreviewScene.GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
	}
}

FBox FUnLive2DAnimBaseViewportClient::GetDesiredFocusBounds() const
{
	if (AnimatedRenderComponent.IsValid())
	{
		return FBox(AnimatedRenderComponent->CalcLocalBounds().GetBox().Min / 2.f, AnimatedRenderComponent->CalcLocalBounds().GetBox().Max / 2.f);
	}

	return FUnLive2DViewportClient::GetDesiredFocusBounds();
}
