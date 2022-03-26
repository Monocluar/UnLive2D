
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UnLive2DAnimNodeBase.generated.h"


USTRUCT()
struct FUnLive2DAnimNode_Base
{
	GENERATED_USTRUCT_BODY()

public:

	virtual void OverrideAsset(class UUnLive2DMotion* NewAsset){};
};