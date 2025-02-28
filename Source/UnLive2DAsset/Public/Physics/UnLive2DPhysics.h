#pragma once

#include "CoreMinimal.h"
#include "Physics/UnLive2DPhysicsInternal.h"

#include "UnLive2DPhysics.generated.h"

// Live2D 物理解算
UCLASS(Blueprintable, BlueprintType, Category = Live2DPhysics, hidecategories = Object)
class UNLIVE2DASSET_API UUnLive2DPhysics : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	// 默认重力方向
	UPROPERTY(EditAnywhere, Category = "Physics")
	FVector2D DefaultGravity;

	// 默认风力方向
	UPROPERTY(EditAnywhere, Category = "Physics")
	FVector2D DefaultWind;


#if WITH_EDITOR
	// 当前编辑器预览
	TWeakPtr<class FUnLive2DRawModel> PreviewLive2DRawModel;
#endif
public:

#if WITH_EDITOR
	bool LoadLive2DPhysicsData(const FString& ReadPhysicsPath);

	void SetLive2DPhysicsData(TArray<uint8>& InPhysicsData);
#endif

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif


protected:

	// <Live2D物理数据
	UPROPERTY()
	TArray<uint8> Live2DPhysicsData;


	friend class UUnLive2D;
};