#pragma once

#include "CoreMinimal.h"
#include "CubismConfig.generated.h"

class UTextureRenderTarget2D;

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

	// 动作数据
	UPROPERTY()
		TArray<uint8> MotionByteData;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
		FString PathName;
#endif // WITH_EDITORONLY_DATA


public:

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

	// <Live2DPose数据
	UPROPERTY()
		TArray<uint8> Live2DPoseData;

	// <Live2D用户数据
	UPROPERTY()
		TArray<uint8> Live2DUserDataData;
};
