
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UnLive2DRendererComponent.h"

#include "UnLive2DAnimInstance.generated.h"

UCLASS(transient, Blueprintable, hideCategories=UnLive2DAnimInstance, BlueprintType, meta=(BlueprintThreadSafe))
class UNLIVE2DASSET_API UUnLive2DAnimInstance : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	virtual void PostEditUndo() override;
#endif

public:

	FORCEINLINE UUnLive2DRendererComponent* GetUnLive2DRendererComponent() const { return CastChecked<UUnLive2DRendererComponent>(GetOuter()); }

public:
#if WITH_EDITORONLY_DATA


	// Called on the newly created CDO during anim blueprint compilation to allow subclasses a chance to replace animations (experimental)
	virtual void ApplyAnimOverridesToCDO(FCompilerResultsLog& MessageLog) {}
#endif
};