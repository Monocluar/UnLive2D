#include "UnLive2DMotionViewportClient.h"
#include "UnLive2DRendererComponent.h"
#include "Animation/UnLive2DAnimBase.h"

FUnLive2DAnimBaseViewportClient::FUnLive2DAnimBaseViewportClient(TWeakObjectPtr<UUnLive2DAnimBase> InUnLive2DAnimBaseBegingEdited)
{
	UnLive2DAnimBaseBeingEditedLastFrame = InUnLive2DAnimBaseBegingEdited;
	PreviewScene = &OwnedPreviewScene;

	SetRealtime(true);

	if (InUnLive2DAnimBaseBegingEdited.IsValid())
	{
		AnimatedRenderComponent = NewObject<UUnLive2DRendererComponent>();
		AnimatedRenderComponent->SetUnLive2D(InUnLive2DAnimBaseBegingEdited->UnLive2D);
		PreviewScene->AddComponent(AnimatedRenderComponent.Get(), FTransform::Identity);
	}

	DrawHelper.bDrawGrid = false;

	EngineShowFlags.DisableAdvancedFeatures();
	EngineShowFlags.SetCompositeEditorPrimitives(true);
}

FUnLive2DAnimBaseViewportClient::~FUnLive2DAnimBaseViewportClient()
{
	if (AnimatedRenderComponent.IsValid())
	{
		AnimatedRenderComponent->DestroyComponent();
	}
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
