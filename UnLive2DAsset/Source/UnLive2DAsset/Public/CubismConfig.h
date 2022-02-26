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
	None,
	Idle,
	Normal,
	TapBody
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

	// <Live2D动作组数据
	UPROPERTY()
		TMap<FName, FUnLiveByteData> Live2DMotionData;

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