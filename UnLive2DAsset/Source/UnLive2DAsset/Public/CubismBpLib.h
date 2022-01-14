#pragma once

#include "CoreMinimal.h"

class UCubismAsset;
class UTexture2D;

/**
 * 
 */
class UNLIVE2DASSET_API UCubismBpLib 
{
	
public:
	// 初始化Live2D SDK
	static void InitCubism();

	// 获取自身项目路径
	static FString GetCubismPath();
};
