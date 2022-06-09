#include "UnLive2DActor.h"
#include "UnLive2DRendererComponent.h"

AUnLive2DActor::AUnLive2DActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	UnLive2DComponent = CreateDefaultSubobject<UUnLive2DRendererComponent>(TEXT("UnLive2DRendererComponent0"));

	RootComponent = UnLive2DComponent;
}

