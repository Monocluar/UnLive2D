#pragma once

#include "CoreMinimal.h"
#include "CubismConfig.generated.h"

class UTextureRenderTarget2D;

UENUM(BlueprintType)
enum class EUnLive2DMotionGroup : uint8
{
	None = 0,
	Idle,
	Normal,
	TapBody
};


UENUM(BlueprintType)
enum class EUnLive2DRenderType : uint8
{
	Mesh,
	RenderTarget,
};

USTRUCT(BlueprintType)
struct FUnLive2DMotionData
{
	GENERATED_BODY()

public:
	// 动作开始时淡入淡出处理时间
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Anim Data")
		float FadeInTime;

	// 动作结束时淡入淡出处理时间
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Anim Data")
		float FadeOutTime;

	// 动作组名称
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Anim Data")
		EUnLive2DMotionGroup MotionGroupType;

	// 该动作组ID
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Anim Data")
		int32 MotionCount;

	// 动作数据
	UPROPERTY()
		TArray<uint8> MotionByteData;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
		FString PathName;
#endif // WITH_EDITORONLY_DATA


public:

	FString GetMotionName() const
	{
		UEnum* GroupEnumPtr = StaticEnum<EUnLive2DMotionGroup>();
		FString GroupStr = GroupEnumPtr->GetNameStringByIndex((int32)MotionGroupType);
		return FString::Printf(TEXT("%s_%d"), *GroupStr, MotionCount);
	}

#if WITH_EDITOR

	FString GetFPathName() const { return PathName; }
#endif // WITH_EDITOR


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
