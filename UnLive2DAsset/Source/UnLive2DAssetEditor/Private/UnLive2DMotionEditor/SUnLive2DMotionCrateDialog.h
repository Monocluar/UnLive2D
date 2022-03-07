
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SWindow;
class UUnLive2DMotionFactory;
class SVerticalBox;
class UUnLive2DMotionImportUI;

class SUnLive2DMotionCrateDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DMotionCrateDialog)
		: _ImportUI(nullptr)
		{}

	SLATE_ARGUMENT(UUnLive2DMotionImportUI*, ImportUI)
	SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs);

	bool ConfigureProperties(TWeakObjectPtr<UUnLive2DMotionFactory> InUnLive2DMotionFactory);

private:

	// 点击确定
	FReply OkClicked();

	// 点击取消
	FReply CancelClicked();

	// 关闭窗口
	void CloseDialog(bool bWasPicked=false);

	// 为父类创建组合菜单
	//void MakeParentClassPicker();

	// 为目标UnLive2D创建组合菜单
	void MakeMotionPicker();

	// <为目标UnLive2D Montion设置参数
	void MakeMotionDetail();

private:

	void OnUnLive2DSelected(const FAssetData& AssetData);

	bool FilterMotionBasedOnParentClass(const FAssetData& AssetData);

	// 是否含有该资源
	bool HasUnLive2DMotion(int32 MotionCount, EUnLive2DMotionGroup MotionGroupType);

private:
	// 工厂类指针
	TWeakObjectPtr<UUnLive2DMotionFactory> UnLive2DMotionFactory;

	/** 指向要求用户选择父类的窗口的指针 */
	TWeakPtr<SWindow> PickerWindow;

	// UnLive2D数据资源选择框
	TSharedPtr<SVerticalBox> UnLive2DContainer;

	// UnLive2D参数填写框
	TSharedPtr<SVerticalBox> UnLive2DMotionPames;

	/** 选定的类 */
	TWeakObjectPtr<UClass> ParentClass;

	/** 选择的UnLive2D资源 */
	FAssetData TargetUnLive2DAsset;

	// 点击确定
	bool bOkClicked;

	// 设置
	UUnLive2DMotionImportUI* ImportUI;

	// UnLive2D含有的动作组
	TArray<FAssetData> MotionAssetArr;
};