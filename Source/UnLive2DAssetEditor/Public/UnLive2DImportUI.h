
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

	// 是否导入物理解算
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ImportSetting)
		bool bIsImportPhysics;

	// 是否导入动作
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ImportSetting)
		bool bIsImportMotion;

	// 是否导入表情
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImportSetting)
		bool bIsImportExpression;

	// 是否导入展示信息
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ImportSetting)
		bool bIsImportDisplayInfo;

public:
	UFUNCTION(BlueprintCallable, Category = Miscellaneous)
		void ResetToDefault();
};