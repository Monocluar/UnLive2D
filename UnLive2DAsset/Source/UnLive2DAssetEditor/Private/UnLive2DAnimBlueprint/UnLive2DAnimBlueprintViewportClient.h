#pragma once

#include "CoreMinimal.h"
#include "UnLive2DViewportClient.h"

class UUnLive2DRendererComponent;
class UUnLive2DAnimBlueprint;

class FUnLive2DAnimBlueprintViewportClient : public FUnLive2DViewportClient
{
public:
	FUnLive2DAnimBlueprintViewportClient(TWeakObjectPtr<UUnLive2DAnimBlueprint> InUnLive2DAnimBlueprintEdited);

	virtual ~FUnLive2DAnimBlueprintViewportClient();

protected:

	virtual void Tick(float DeltaSeconds) override;


	virtual FBox GetDesiredFocusBounds() const override;

private:

	TWeakObjectPtr<UUnLive2DAnimBlueprint> UnLive2DAnimBlueprintEditedLastFrame;

	TWeakObjectPtr<UUnLive2DRendererComponent> AnimatedRenderComponent;
private:
	// The preview scene
	FPreviewScene OwnedPreviewScene;
};