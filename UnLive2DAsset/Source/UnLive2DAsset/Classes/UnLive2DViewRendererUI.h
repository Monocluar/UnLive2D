
#pragma once


#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Components/Widget.h"
#include "UnLive2DViewRendererUI.generated.h"

class SUnLive2DViewUI;

UCLASS()
class UNLIVE2DASSET_API UUnLive2DViewRendererUI : public UWidget
{
	GENERATED_UCLASS_BODY()


public:
	// UnLive2D资源
	UPROPERTY(Category = Live2D, EditAnywhere)
		UUnLive2D* SourceUnLive2D;

public:
	// Live2D颜色混合模式为CubismBlendMode_Normal使用的材质
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Rendering)
		UMaterialInterface* UnLive2DNormalMaterial;

	// Live2D颜色混合模式为CubismBlendMode_Additive使用的材质
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Rendering)
		UMaterialInterface* UnLive2DAdditiveMaterial;

	// Live2D颜色混合模式为CubismBlendMode_Multiplicative使用的材质
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Rendering)
		UMaterialInterface* UnLive2DMultiplyMaterial;

	// 材质读取图片参数名
	UPROPERTY(Category = Rendering, EditAnywhere, BlueprintReadWrite)
		FName TextureParameterName;

protected:
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

protected:

	virtual TSharedRef<SWidget> RebuildWidget() override;

	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

protected:

	TSharedPtr<SUnLive2DViewUI> MySlateWidget;

};