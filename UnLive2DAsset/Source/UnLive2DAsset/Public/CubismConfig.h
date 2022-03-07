#pragma once

#include "CoreMinimal.h"
#include "CubismConfig.generated.h"

/**
 * 模型渲染配置
 */
USTRUCT(BlueprintType)
struct FModelConfig
{
	GENERATED_USTRUCT_BODY()

public:
	FModelConfig(){};

public:

	/**
	 * 如果可能，我们将尝试在单个纹理中渲染所有遮罩
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bTryLowPreciseMask = true;

};

UENUM(BlueprintType)
enum class EUnLive2DMotionGroup : uint8
{
	None = 0,
	Idle,
	Normal,
	TapBody
};

USTRUCT(BlueprintType)
struct FUnLive2DMotionData
{
	GENERATED_USTRUCT_BODY()

public:
	// 动作开始时淡入淡出处理时间
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float FadeInTime;

	// 动作结束时淡入淡出处理时间
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float FadeOutTime;

	// 动作组名称
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		EUnLive2DMotionGroup MotionGroupType;

	// 该动作组ID
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int32 MotionCount;

	// 动作数据
	UPROPERTY()
		TArray<uint8> MotionByteData;

public:
	FUnLive2DMotionData()
	{}

	FString GetMotionName() const
	{
		UEnum* GroupEnumPtr = StaticEnum<EUnLive2DMotionGroup>();
		FString GroupStr = GroupEnumPtr->GetNameStringByIndex((int32)MotionGroupType);
		return FString::Printf(TEXT("%s_%d"), *GroupStr, MotionCount);
	}

};

USTRUCT()
struct FUnLiveByteData
{
	GENERATED_USTRUCT_BODY()

public:
	FUnLiveByteData()
	{}

	FUnLiveByteData(TArray<uint8>& Temp)
		: ByteData(MoveTemp(Temp))
	{}

public:
	UPROPERTY()
		TArray<uint8> ByteData;
};

USTRUCT()
struct FUnLive2DLoadData
{
	GENERATED_USTRUCT_BODY()

public:
	// <Live2D模型数据
	UPROPERTY()
		TArray<uint8> Live2DModelData;

	// <Live2D立体数据
	UPROPERTY()
		TArray<uint8> Live2DCubismData;

	// <Live2D表情数据
	UPROPERTY()
		TMap<FName, FUnLiveByteData> Live2DExpressionData;

	/*// <Live2D动作组数据
	UPROPERTY()
		TMap<FName, FUnLiveByteData> Live2DMotionData;*/

	// <Live2D物理数据
	UPROPERTY()
		TArray<uint8> Live2DPhysicsData;

	// <Live2DPose数据
	UPROPERTY()
		TArray<uint8> Live2DPoseData;

	// <Live2D用户数据
	UPROPERTY()
		TArray<uint8> Live2DUserDataData;
};