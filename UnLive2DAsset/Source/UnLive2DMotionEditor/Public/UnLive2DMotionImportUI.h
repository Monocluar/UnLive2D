
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "CubismConfig.h"

#include "UnLive2DMotionImportUI.generated.h"

UCLASS(BlueprintType, AutoExpandCategories = (FTransform), HideCategories = Object, MinimalAPI)
class UUnLive2DMotionImportUI : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImportSetting)
		EUnLive2DMotionGroup MotionGroupType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImportSetting)
		int32 MotionCount;

	UPROPERTY(VisibleAnywhere)
		FName AssetPath;

public:
	UFUNCTION(BlueprintCallable, Category = Miscellaneous)
		void ResetToDefault();
};