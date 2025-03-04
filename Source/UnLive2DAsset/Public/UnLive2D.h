// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CubismConfig.h"
#include "Tickable.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "UnLive2D.generated.h"

class FUnLive2DRawModel;
class UMaterialInterface;
class UBodySetup;
class UTexture2D;
class UUnLive2DMotion;
class UUnLive2DExpression;


UCLASS(Blueprintable, BlueprintType, Category = UnLive2D, hidecategories=Object)
class UNLIVE2DASSET_API UUnLive2D : public UObject
{
    GENERATED_UCLASS_BODY()

#if WITH_EDITOR
public:
		DECLARE_DELEGATE_OneParam(FOnUpDataUnLive2DProperty, FName);
#endif
public:

	// 对资源模型进行缩放
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Live2D)
		uint8 Live2DScale;

	// 贴图资源
	UPROPERTY(BlueprintReadWrite, EditAnywhere, AssetRegistrySearchable, Category = Live2D)
		TArray<UTexture2D*> TextureAssets;

	// 物理解算
	UPROPERTY(BlueprintReadWrite, EditAnywhere, AssetRegistrySearchable, Category = Physics)
		class UUnLive2DPhysics* Live2DPhysics;

#if WITH_EDITORONLY_DATA
		// 展示信息
	UPROPERTY(BlueprintReadWrite, EditAnywhere, AssetRegistrySearchable, Category = Display)
		class UEditorUnLive2DDisplayInfo* Live2DDisplayInfo;
#endif

public:

	/*// 是否使用单遮罩图（ 为false 遮罩图不更新）
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Rendering)
		bool bTryLowPreciseMask = true;*/

	/** 此组件的着色颜色和不透明度 */
	UPROPERTY(EditAnywhere, Category = Rendering)
		FLinearColor TintColorAndOpacity;

public:

#if WITH_EDITOR
	// 更新UnLive2D其他参数
	FOnUpDataUnLive2DProperty OnUpDataUnLive2DProperty;
#endif

public:

	// 创建Live2D模型数据
	TSharedPtr<FUnLive2DRawModel> CreateLive2DRawModel() const;

#if WITH_EDITOR
	struct FOtherExportData
	{
		FString PhysicsPath; // 物理路径
		FString DisplayInfoPath; // 展示信息
	};

	// 加载Live2D文件数据
	void LoadLive2DFileDataFormPath(const FString& InPath, TArray<FString>& TexturePaths, TArray<FUnLive2DMotionData>& LoadMotionData, TMap<FString, FUnLiveByteData>& LoadExpressionData, FOtherExportData& OutExportData);

#endif
	const FUnLive2DLoadData* GetUnLive2DLoadData();

public:

	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

#endif

protected:

	// 基础数据
	UPROPERTY()
		FUnLive2DLoadData Live2DFileData;

private:

#if WITH_EDITOR
	mutable TArray<TWeakPtr<FUnLive2DRawModel>> CacheRawModelArr;
#endif
};