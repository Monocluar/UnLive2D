#include "Animation/UnLive2DMotionPlayCallbackProxy.h"
#include "UnLive2DRendererComponent.h"
#include "UnLive2DViewRendererUI.h"

UUnLive2DMotionPlayCallbackProxy::UUnLive2DMotionPlayCallbackProxy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UUnLive2DMotionPlayCallbackProxy* UUnLive2DMotionPlayCallbackProxy::CreateProxyObjectForPlayMotion_RendererComponent(class UUnLive2DRendererComponent* InUnLive2DRendererComponent, class UUnLive2DMotion* MontageToPlay)
{
	UUnLive2DMotionPlayCallbackProxy* Proxy = NewObject<UUnLive2DMotionPlayCallbackProxy>();
	Proxy->SetFlags(RF_StrongRefOnFrame);
	Proxy->PlayMotion(InUnLive2DRendererComponent, MontageToPlay);
	return Proxy;
}

UUnLive2DMotionPlayCallbackProxy* UUnLive2DMotionPlayCallbackProxy::CreateProxyObjectForPlayMotion_ViewRendererUI(class UUnLive2DViewRendererUI* InUnLive2DViewRendererUI, UUnLive2DMotion* MontageToPlay)
{
	UUnLive2DMotionPlayCallbackProxy* Proxy = NewObject<UUnLive2DMotionPlayCallbackProxy>();
	Proxy->SetFlags(RF_StrongRefOnFrame);
	Proxy->PlayMotion(InUnLive2DViewRendererUI, MontageToPlay);
	return Proxy;
}

void UUnLive2DMotionPlayCallbackProxy::BeginDestroy()
{
	UnbindDelegates();
	Super::BeginDestroy();
}

void UUnLive2DMotionPlayCallbackProxy::OnMontageEnded(UUnLive2DMotion* Montage, bool bInterrupted)
{
	if (UnLive2DMotionPtr.Get() != Montage) return;
	
	if (bInterrupted)
	{
		OnInterrupted.Broadcast();
	}
	else
	{
		OnCompleted.Broadcast();
	}

	UnbindDelegates();
}

void UUnLive2DMotionPlayCallbackProxy::PlayMotion(class UUnLive2DRendererComponent* InUnLive2DRendererComponent, class UUnLive2DMotion* MontageToPlay)
{
	bool bPlayedSuccessfully = false;

	if (InUnLive2DRendererComponent)
	{
		UnLive2DMotionPtr = MontageToPlay;
		UnLive2DMotionEnded.BindUObject(this, &UUnLive2DMotionPlayCallbackProxy::OnMontageEnded);
		UnLive2DMotionPtr->OnUnLive2DMotionEnded = UnLive2DMotionEnded;

		InUnLive2DRendererComponent->PlayMotion(MontageToPlay);
		bPlayedSuccessfully = true;
	}
	
	if (!bPlayedSuccessfully)
	{
		OnInterrupted.Broadcast();
	}
}

void UUnLive2DMotionPlayCallbackProxy::PlayMotion(class UUnLive2DViewRendererUI* InUnLive2DViewRendererUI, UUnLive2DMotion* MontageToPlay)
{
	bool bPlayedSuccessfully = false;

	if (InUnLive2DViewRendererUI)
	{
		UnLive2DMotionPtr = MontageToPlay;
		UnLive2DMotionEnded.BindUObject(this, &UUnLive2DMotionPlayCallbackProxy::OnMontageEnded);
		UnLive2DMotionPtr->OnUnLive2DMotionEnded = UnLive2DMotionEnded;

		InUnLive2DViewRendererUI->PlayMotion(MontageToPlay);
		bPlayedSuccessfully = true;
	}

	if (!bPlayedSuccessfully)
	{
		OnInterrupted.Broadcast();
	}
}

void UUnLive2DMotionPlayCallbackProxy::UnbindDelegates()
{
	if (UnLive2DMotionPtr.IsValid())
	{
		//UnLive2DMotionPtr->OnUnLive2DMotionEnded
	}
}

