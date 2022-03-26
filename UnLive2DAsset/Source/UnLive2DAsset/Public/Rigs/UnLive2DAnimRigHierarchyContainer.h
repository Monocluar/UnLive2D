
#pragma once

#include "CoreMinimal.h"
#include "UnLive2DAnimRigHierarchyContainer.generated.h"


USTRUCT(BlueprintType)
struct UNLIVE2DASSET_API FUnLive2DAnimRigHierarchyContainer
{
	GENERATED_BODY()

public:
	FUnLive2DAnimRigHierarchyContainer();

public:

	UPROPERTY(transient)
		int32 Version;

private:

	void Initialize(bool bResetTransforms = true);
};