
#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "UnLive2DEditorStyle.h"


class FUnLive2DAnimBlueprintEditorCommands : public TCommands<FUnLive2DAnimBlueprintEditorCommands>
{
public:
	FUnLive2DAnimBlueprintEditorCommands()
		: TCommands<FUnLive2DAnimBlueprintEditorCommands>("UnLive2DAnimBlueprintGraphEditor", NSLOCTEXT("Contexts", "UnLive2DAnimBlueprintGraphEditor", "UnLive2D AnimBlueprint GraphEditor"), NAME_None, FUnLive2DEditorStyle::GetStyleSetName())
	{}

public:
	// 播放动画蓝图
	TSharedPtr<FUICommandInfo> PlayUnLive2DAnimBlueprint;

	// 播放节点数据动画
	TSharedPtr<FUICommandInfo> PlayNode;

	// 暂停播放动画
	TSharedPtr<FUICommandInfo> StopUnLive2DAnim;

	// 在内容浏览器中选择动画资源
	TSharedPtr<FUICommandInfo> BrowserSync;

	// 删除一个输入节点
	TSharedPtr<FUICommandInfo> DeleteInput;

public:

	virtual void RegisterCommands() override;
};