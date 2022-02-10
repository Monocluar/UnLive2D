
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"

#include "UnLive2DImportUI.generated.h"

UCLASS(BlueprintType, config=EditorPerProjectUserSettings, AutoExpandCategories=(FTransform), HideCategories=Object, MinimalAPI)
class UUnLive2DImportUI : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	// 是否导入图片
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ImportSetting)
		bool bIsImportTexture;

public:
	UFUNCTION(BlueprintCallable, Category = Miscellaneous)
		void ResetToDefault();
};