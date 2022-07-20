#pragma once

#include "CoreMinimal.h"
#include "UnLive2DViewportClient.h"

class UUnLive2DRendererComponent;
class UUnLive2DAnimBase;
class SUnLive2DAnimBaseEditorViewport;

class FUnLive2DAnimBaseViewportClient : public FUnLive2DViewportClient
{
public:
	FUnLive2DAnimBaseViewportClient(TWeakObjectPtr<UUnLive2DAnimBase> InUnLive2DAnimBaseBegingEdited);

protected:

	virtual void Tick(float DeltaSeconds) override;


	virtual FBox GetDesiredFocusBounds() const override;

private:

	TWeakObjectPtr<UUnLive2DAnimBase> UnLive2DAnimBaseBeingEditedLastFrame;

};