// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UUnLive2D;
class UUnLive2DRendererComponent;
class UUnLive2DAnimBlueprint;
class UUnLive2DAnimBase;

class IUnLive2DToolkit
{
public:
	virtual ~IUnLive2DToolkit() {}

public:
	/** 获取Live2D资源 */
	virtual UUnLive2D* GetUnLive2D() const = 0;

	/** 获取Live2D浏览组件 */
	virtual UUnLive2DRendererComponent* GetPreviewUnLive2DComponent() const = 0;

	// 获取Live2D动画蓝图
	virtual UUnLive2DAnimBlueprint* GetAnimBlueprint() const = 0;

	// 获取Live2D动画资源
	virtual UUnLive2DAnimBase* GetAnimBaseAsset() const = 0;

	/** 检索编辑器自定义数据。如果密钥无效，则返回INDEX_NONE */
	virtual int32 GetCustomData(const int32 Key) const { return INDEX_NONE; }

	/** 使用密钥存储自定义数据。 */
	virtual void SetCustomData(const int32 Key, const int32 CustomData) {}
	
	/** 使用此工具包的上下文（通常是资产的类名） */
	virtual FName GetContext() const = 0;
};

