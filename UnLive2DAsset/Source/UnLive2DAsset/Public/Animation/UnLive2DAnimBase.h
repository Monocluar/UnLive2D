
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "UnLive2D.h"
#include "UnLive2DAnimBase.generated.h"


UCLASS(hideCategories=Object, Abstract, editinlinenew, BlueprintType)
class UNLIVE2DASSET_API UUnLive2DAnimBase : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	// UnLive2D设置
	UPROPERTY(AssetRegistrySearchable, Category = Animation, VisibleAnywhere)
		UUnLive2D* UnLive2D;

public:	

	virtual void Parse(struct FActiveUnLive2DAnimBlueprint& ActiveLive2DAnim, struct FUnLive2DAnimParseParameters& ParseParams, const UPTRINT NodeAnimInstanceHash){};
};