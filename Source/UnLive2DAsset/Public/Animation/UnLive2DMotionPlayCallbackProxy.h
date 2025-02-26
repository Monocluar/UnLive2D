
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "UObject/ScriptMacros.h"
#include "UnLive2DMotion.h"
#include "UnLive2DMotionPlayCallbackProxy.generated.h"

class UUnLive2DMotion;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUnLive2DMotionResult);

UCLASS(MinimalAPI)
class UUnLive2DMotionPlayCallbackProxy : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	// 播放完成，并且没有被打断
	UPROPERTY(BlueprintAssignable)
		FUnLive2DMotionResult OnCompleted;

	// 播放被打断
	UPROPERTY(BlueprintAssignable)
		FUnLive2DMotionResult OnInterrupted;

public:
	// Called to perform the query internally
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
		static UUnLive2DMotionPlayCallbackProxy* CreateProxyObjectForPlayMotion_RendererComponent( class UUnLive2DRendererComponent* InUnLive2DRendererComponent, UUnLive2DMotion* MontageToPlay);

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
		static UUnLive2DMotionPlayCallbackProxy* CreateProxyObjectForPlayMotion_ViewRendererUI(class UUnLive2DViewRendererUI* InUnLive2DViewRendererUI, UUnLive2DMotion* MontageToPlay);

protected:

	virtual void BeginDestroy() override;

private:

	UFUNCTION()
		void OnMontageEnded(UUnLive2DMotion* Montage, bool bInterrupted);

private:

	void PlayMotion(class UUnLive2DRendererComponent* InUnLive2DRendererComponent, UUnLive2DMotion* MontageToPlay);
	void PlayMotion(class UUnLive2DViewRendererUI* InUnLive2DViewRendererUI, UUnLive2DMotion* MontageToPlay);

	void UnbindDelegates();

private:
	TWeakObjectPtr<UUnLive2DMotion> UnLive2DMotionPtr;

	FOnUnLive2DMotionEnded UnLive2DMotionEnded;

};