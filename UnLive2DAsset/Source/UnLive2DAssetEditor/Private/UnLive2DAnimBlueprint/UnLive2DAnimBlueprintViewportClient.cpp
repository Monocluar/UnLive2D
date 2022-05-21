#include "UnLive2DAnimBlueprintViewportClient.h"
#include "UnLive2DRendererComponent.h"
#include "Animation/UnLive2DAnimBlueprint.h"

FUnLive2DAnimBlueprintViewportClient::FUnLive2DAnimBlueprintViewportClient(TWeakObjectPtr<UUnLive2DAnimBlueprint> InUnLive2DAnimBlueprintEdited)
{
	UnLive2DAnimBlueprintEditedLastFrame = InUnLive2DAnimBlueprintEdited;
	PreviewScene = &OwnedPreviewScene;

	SetRealtime(true);

	if (InUnLive2DAnimBlueprintEdited.IsValid())
	{
		AnimatedRenderComponent = NewObject<UUnLive2DRendererComponent>();
		AnimatedRenderComponent->SetUnLive2D(InUnLive2DAnimBlueprintEdited->TargetUnLive2D);
		PreviewScene->AddComponent(AnimatedRenderComponent.Get(), FTransform::Identity);
	}

	DrawHelper.bDrawGrid = false;

	EngineShowFlags.DisableAdvancedFeatures();
	EngineShowFlags.SetCompositeEditorPrimitives(true);
}

FUnLive2DAnimBlueprintViewportClient::~FUnLive2DAnimBlueprintViewportClient()
{
	if (AnimatedRenderComponent.IsValid())
	{
		AnimatedRenderComponent->DestroyComponent();
	}
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
