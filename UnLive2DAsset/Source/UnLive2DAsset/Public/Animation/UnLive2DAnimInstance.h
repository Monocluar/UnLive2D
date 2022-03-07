
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "UnLive2DAnimInstance.generated.h"

UCLASS(Blueprintable, Abstract, editinlinenew)
class UNLIVE2DASSET_API UUnLive2DAnimInstance : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	virtual void PostEditUndo() override;
#endif


private:
};