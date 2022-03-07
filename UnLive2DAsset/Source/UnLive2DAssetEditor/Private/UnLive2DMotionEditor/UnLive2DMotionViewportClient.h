#pragma once

#include "CoreMinimal.h"
#include "UnLive2DViewportClient.h"

class UUnLive2DRendererComponent;
class UUnLive2DMotion;

class FUnLive2DMotionViewportClient : public FUnLive2DViewportClient
{
public:
	FUnLive2DMotionViewportClient(TWeakObjectPtr<UUnLive2DMotion> InUnLive2DMotionBegingEdited);

	virtual ~FUnLive2DMotionViewportClient();

protected:

	virtual void Tick(float DeltaSeconds) override;

private:

	TWeakObjectPtr<UUnLive2DMotion> UnLive2DMotionBeingEditedLastFrame;

	TWeakObjectPtr<UUnLive2DRendererComponent> AnimatedRenderComponent;
private:
	// The preview scene
	FPreviewScene OwnedPreviewScene;
};