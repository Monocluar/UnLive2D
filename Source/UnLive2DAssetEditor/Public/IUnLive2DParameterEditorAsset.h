#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"
#include "EditorUndoClient.h"

class UUnLive2DRendererComponent;
class UUnLive2D;

namespace EUnLive2DParameterAssetType
{
	enum Type
	{
		UnLive2D, //ul2d模型数据

		UnLive2DExpression, // 表情数据

		UnLive2DMotion, //动画数据

		UnLive2DAnimBlueprint, // 动画蓝图
	};
}

class IUnLive2DParameterEditorAsset : public FWorkflowCentricApplication, public FGCObject, public FEditorUndoClient
{
	
public:
	
	// UnLive2D资源
	virtual UUnLive2D* GetUnLive2DBeingEdited() const = 0;

	// UnLive2D界面模拟查看组件
	virtual TWeakObjectPtr<UUnLive2DRendererComponent> GetUnLive2DRenderComponent() const = 0;

	// UnLive2D界面参数界面类型
	virtual EUnLive2DParameterAssetType::Type GetUnLive2DParameterAssetType() const = 0;

	// 是否可以保存更改数据
	virtual bool GetUnLive2DParameterHasSaveData() const { return false; }

	// 是否可以添加数据
	virtual bool GetUnLive2DParameterAddParameterData() const { return false; }

};