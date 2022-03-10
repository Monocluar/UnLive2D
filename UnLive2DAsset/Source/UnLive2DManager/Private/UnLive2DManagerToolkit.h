// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IUnLive2DToolkit.h"

class UUnLive2D;
class UUnLive2DMotion;
class UUnLive2DAnimBlueprint;


class FUnLive2DMangerToolkit : public IUnLive2DToolkit, public TSharedFromThis<FUnLive2DMangerToolkit> 
{
public:
	FUnLive2DMangerToolkit();

	virtual ~FUnLive2DMangerToolkit() {}

public:

	void Initialize(UUnLive2D* InUnLive2D);
	void Initialize(UUnLive2DMotion* InUnLive2DMotion);
	void Initialize(UUnLive2DAnimBlueprint* InAnimBlueprint);

	// 创建预览窗口
	void CreatePreviewScene();

public:	
	/** 获取Live2D资源 */
	virtual UUnLive2D* GetUnLive2D() const override;

	/** 获取Live2D浏览组件 */
	virtual UUnLive2DRendererComponent* GetPreviewUnLive2DComponent() const override;

	// 获取Live2D动画蓝图
	virtual UUnLive2DAnimBlueprint* GetAnimBlueprint() const override;

	// 获取Live2D动画资源
	virtual UUnLive2DMotion* GetMotionAsset() const override;

	/** 检索编辑器自定义数据。如果密钥无效，则返回INDEX_NONE */
	virtual int32 GetCustomData(const int32 Key) const override;

	/** 使用密钥存储自定义数据。 */
	virtual void SetCustomData(const int32 Key, const int32 CustomData) override;
	
	/** 使用此工具包的上下文（通常是资产的类名） */
	virtual FName GetContext() const override;

private:
	// Live2D资源
	TWeakObjectPtr<UUnLive2D> UnLive2D;

	// 动画资源
	TWeakObjectPtr<UUnLive2DMotion> UnLive2DMotion;

	// 动画蓝图
	TWeakObjectPtr<UUnLive2DAnimBlueprint> UnLive2DAnimBlueprint;

	/** 允许此编辑器使用自定义数据 */
	TMap<int32, int32> CustomEditorData;

	/** 我们创建时使用的初始资产的类别 */
	UClass* InitialAssetClass;
};