
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

	// 动作开始时淡入淡出处理时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImportSetting)
		float FadeInTime;

	// 动作结束时淡入淡出处理时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImportSetting)
		float FadeOutTime;

	UPROPERTY(VisibleAnywhere)
		FName AssetPath;

public:
	UFUNCTION(BlueprintCallable, Category = Miscellaneous)
		void ResetToDefault();
};