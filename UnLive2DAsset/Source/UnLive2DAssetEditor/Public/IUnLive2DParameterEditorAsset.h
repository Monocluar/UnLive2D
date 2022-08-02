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
	};
}

class IUnLive2DParameterEditorAsset : public FWorkflowCentricApplication, public FGCObject, public FEditorUndoClient
{
	
public:

	virtual UUnLive2D* GetUnLive2DBeingEdited() const = 0;

	virtual TWeakObjectPtr<UUnLive2DRendererComponent> GetUnLive2DRenderComponent() const = 0;

	virtual EUnLive2DParameterAssetType::Type GetUnLive2DParameterAssetType() const = 0;
};