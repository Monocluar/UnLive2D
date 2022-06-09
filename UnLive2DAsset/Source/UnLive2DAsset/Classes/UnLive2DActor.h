
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "GameFramework/Actor.h"
#include "UnLive2DActor.generated.h"

class UUnLive2DRendererComponent;

UCLASS(MinimalAPI, hideCategories = (Activation, "Components|Activation", Input, Collision, "Game|Damage"), ComponentWrapperClass)
class AUnLive2DActor : public AActor
{
	GENERATED_UCLASS_BODY()


private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=UnLive2DActor, meta = (AllowPrivateAccess = "true"))
		UUnLive2DRendererComponent* UnLive2DComponent;

public:

	UNLIVE2DASSET_API UUnLive2DRendererComponent* GetUnLive2DRendererComponent() const { return UnLive2DComponent; }
};