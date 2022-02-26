
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "CubismConfig.h"
#include "UnLive2DMotion.generated.h"

class UUnLive2D;

UCLASS(Blueprintable, BlueprintType, Category = UnLive2DMotion)
class UNLIVE2DASSET_API UUnLive2DMotion : public UObject
{
	GENERATED_BODY()

public:
	// UnLive2D设置
	UPROPERTY(AssetRegistrySearchable, Category = Animation, VisibleAnywhere)
		UUnLive2D* UnLive2D;

public:

#if WITH_EDITOR
	bool LoadLive2DMotionData(const FString& ReadMotionPath, EUnLive2DMotionGroup InMotionGroupType, int32 InMotionCount);
#endif


private:

	// 动作组名称
	UPROPERTY()
		EUnLive2DMotionGroup MotionGroupType;

	// 该动作组ID
	UPROPERTY()
		int32 MotionCount;

	// 动作数据
	UPROPERTY()
		TArray<uint8> MotionByteData;
};