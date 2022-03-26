#include "UnLive2DMotionViewportClient.h"
#include "UnLive2DRendererComponent.h"
#include "UnLive2DMotion.h"

FUnLive2DMotionViewportClient::FUnLive2DMotionViewportClient(TWeakObjectPtr<UUnLive2DMotion> InUnLive2DMotionBegingEdited)
{
	UnLive2DMotionBeingEditedLastFrame = InUnLive2DMotionBegingEdited;
	PreviewScene = &OwnedPreviewScene;

	SetRealtime(true);

	if (InUnLive2DMotionBegingEdited.IsValid())
	{
		AnimatedRenderComponent = NewObject<UUnLive2DRendererComponent>();
		AnimatedRenderComponent->SetUnLive2D(InUnLive2DMotionBegingEdited->UnLive2D);
		PreviewScene->AddComponent(AnimatedRenderComponent.Get(), FTransform::Identity);
	}

	DrawHelper.bDrawGrid = false;

	EngineShowFlags.DisableAdvancedFeatures();
	EngineShowFlags.SetCompositeEditorPrimitives(true);
}

FUnLive2DMotionViewportClient::~FUnLive2DMotionViewportClient()
{
	if (AnimatedRenderComponent.IsValid())
	{
		AnimatedRenderComponent->DestroyComponent();
	}
}

void FUnLive2DMotionViewportClient::Tick(float DeltaSeconds)
{
	FUnLive2DViewportClient::Tick(DeltaSeconds);

	if (!GIntraFrameDebuggingGameThread)
	{
		OwnedPreviewScene.GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
	}
}

FBox FUnLive2DMotionViewportClient::GetDesiredFocusBounds() const
{
	if (AnimatedRenderComponent.IsValid())
	{
		return FBox(AnimatedRenderComponent->CalcLocalBounds().GetBox().Min / 2.f, AnimatedRenderComponent->CalcLocalBounds().GetBox().Max / 2.f);
	}

	return FUnLive2DViewportClient::GetDesiredFocusBounds();
}
