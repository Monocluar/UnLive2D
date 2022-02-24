// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IUnLive2DToolkit.h"

class UUnLive2D;


class FUnLive2DMangerToolkit : public IUnLive2DToolkit, public TSharedFromThis<FUnLive2DMangerToolkit> 
{
public:
	FUnLive2DMangerToolkit();

	virtual ~FUnLive2DMangerToolkit() {}

public:

	void Initialize(UUnLive2D* InUnLive2D);

	// 创建预览窗口
	void CreatePreviewScene();

public:	
	/** 获取Live2D资源 */
	virtual UUnLive2D* GetUnLive2D() const override;

	/** 检索编辑器自定义数据。如果密钥无效，则返回INDEX_NONE */
	virtual int32 GetCustomData(const int32 Key) const override;

	/** 使用密钥存储自定义数据。 */
	virtual void SetCustomData(const int32 Key, const int32 CustomData) override;
	
	/** 使用此工具包的上下文（通常是资产的类名） */
	virtual FName GetContext() const override;

private:
	// Live2D资源
	TWeakObjectPtr<UUnLive2D> UnLive2D;

	/** 允许此编辑器使用自定义数据 */
	TMap<int32, int32> CustomEditorData;

	/** 我们创建时使用的初始资产的类别 */
	UClass* InitialAssetClass;
};