#pragma once

#include "CoreMinimal.h"

class UCubismAsset;
class UTexture2D;

/**
 * 
 */
#if WITH_EDITOR
struct UNLIVE2DASSET_API FUnLive2DParameterData
{

public:
	// Live2D参数ID
	const int32 ParameterID;

	// Live2D参数名
	const FName ParameterName;

	// 参数值
	float ParameterValue;

	// 参数默认值
	const float ParameterDefaultValue;

	// 参数最小值
	const float ParameterMinValue;

	// 参数最大值
	const float ParameterMaxValue;


public:
	FUnLive2DParameterData()
		: ParameterID(INT_ERROR)
		, ParameterName()
		, ParameterValue(0.f)
		, ParameterDefaultValue(0.f)
		, ParameterMinValue(0.f)
		, ParameterMaxValue(0.f)
	{}

	FUnLive2DParameterData(
		int32 InParameterID,
		FName InParameterName,
		float InParameterValue,
		float InParameterDefaultValue,
		float InParameterMinValue,
		float InParameterMaxValue
	)
		: ParameterID(InParameterID)
		, ParameterName(InParameterName)
		, ParameterValue(InParameterValue)
		, ParameterDefaultValue(InParameterDefaultValue)
		, ParameterMinValue(InParameterMinValue)
		, ParameterMaxValue(InParameterMaxValue)
	{
	}
};
#endif // WITH_EDITOR


class UNLIVE2DASSET_API UCubismBpLib 
{
	
public:
	// 初始化Live2D SDK
	static void InitCubism();

	// 获取自身项目路径
	static FString GetCubismPath();
};
