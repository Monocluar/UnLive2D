
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "UnLive2DCubismCore.h"
#include "Widgets/Layout/SBorder.h"
#include "UnLive2D.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SWindow.h"
#include "Physics/UnLive2DPhysicsFactory.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetRegistryModule.h"


class SUnLive2DNorCreateDialog : public SCompoundWidget
{

public:
	SLATE_BEGIN_ARGS(SUnLive2DNorCreateDialog)
		{}

	SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs);

	bool ConfigureProperties(TWeakObjectPtr<UFactory> InUnLive2DPhysicsFactory);

private:

	// 点击确定
	FReply OkClicked();

	// 点击取消
	FReply CancelClicked();

	// 关闭窗口
	void CloseDialog(bool bWasPicked = false);

	// 为目标UnLive2D创建组合菜单
	void MakeMotionPicker();

private:

	void OnUnLive2DSelected(const FAssetData& AssetData);

	bool FilterMotionBasedOnParentClass(const FAssetData& AssetData);

private:
	// 工厂类指针
	TWeakObjectPtr<UFactory> UnLive2DPhysicsFactory;

	/** 指向要求用户选择父类的窗口的指针 */
	TWeakPtr<SWindow> PickerWindow;

	// UnLive2D数据资源选择框
	TSharedPtr<SVerticalBox> UnLive2DContainer;

	/** 选定的类 */
	TWeakObjectPtr<UClass> ParentClass;

	/** 选择的UnLive2D资源 */
	FAssetData TargetUnLive2DAsset;

	// 点击确定
	bool bOkClicked;
};

void SUnLive2DNorCreateDialog::Construct(const FArguments& InArgs)
{
	bOkClicked = false;
	
	ChildSlot
	[
		SNew(SBorder)
		.Visibility(EVisibility::Visible)
		.BorderImage(FUnLive2DStyle::GetBrush("Menu.Background"))
		[
			
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.FillHeight(1)
			[
				SNew(SBorder)
				.BorderImage(FUnLive2DStyle::GetBrush("ToolPanel.GroupBorder"))
				.Content()
				[
					SAssignNew(UnLive2DContainer, SVerticalBox)
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(8)
			[
				SNew(SUniformGridPanel)
				.SlotPadding(FUnLive2DStyle::GetMargin("StandardDialog.SlotPadding"))
				.MinDesiredSlotWidth(FUnLive2DStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
				.MinDesiredSlotHeight(FUnLive2DStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))
				+ SUniformGridPanel::Slot(0, 0)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.ContentPadding(FUnLive2DStyle::GetMargin("StandardDialog.ContentPadding"))
					.OnClicked(this, &SUnLive2DNorCreateDialog::OkClicked)
					.Text(NSLOCTEXT("AnimBlueprintFactory", "CreateAnimBlueprintOk", "OK"))
				]
				+ SUniformGridPanel::Slot(1, 0)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.ContentPadding(FUnLive2DStyle::GetMargin("StandardDialog.ContentPadding"))
					.OnClicked(this, &SUnLive2DNorCreateDialog::CancelClicked)
					.Text(NSLOCTEXT("AnimBlueprintFactory", "CreateAnimBlueprintCancel", "Cancel"))
				]
			]
		]
	];

	MakeMotionPicker();
}

bool SUnLive2DNorCreateDialog::ConfigureProperties(TWeakObjectPtr<UFactory> InUnLive2DPhysicsFactory)
{
	UnLive2DPhysicsFactory = InUnLive2DPhysicsFactory;
	
	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(NSLOCTEXT("UnLive2DMotionFactory","CreateUnLive2DPhysicsOptions", "Create UnLive2D Physics Actor"))
		.ClientSize(FVector2D(400, 500))
		.SupportsMinimize(false).SupportsMaximize(false)
		[
			AsShared()
		];

	PickerWindow = Window;

	GEditor->EditorAddModalWindow(Window);
	UnLive2DPhysicsFactory.Reset(); //回收

	return bOkClicked;
}

FReply SUnLive2DNorCreateDialog::OkClicked()
{
	if (!UnLive2DPhysicsFactory.IsValid()) return FReply::Handled();

	if (UUnLive2DPhysicsFactory* PhysicsFactory = Cast<UUnLive2DPhysicsFactory>(UnLive2DPhysicsFactory.Get()))
	{
		PhysicsFactory->TargetUnLive2D = Cast<UUnLive2D>(TargetUnLive2DAsset.GetAsset());
	}

	if (!TargetUnLive2DAsset.IsValid())
	{
		// if TargetSkeleton is not valid
		FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("UnLive2DMotionFactory", "NeedValidUnLive2D", "没有该UnLive2D数据驱动资源"));
		return FReply::Handled();
	}

	CloseDialog(true);

	return FReply::Handled();
}

FReply SUnLive2DNorCreateDialog::CancelClicked()
{
	CloseDialog(false);
	return FReply::Handled();
}

void SUnLive2DNorCreateDialog::CloseDialog(bool bWasPicked /*= false*/)
{
	bOkClicked = bWasPicked;
	if (PickerWindow.IsValid())
	{
		PickerWindow.Pin()->RequestDestroyWindow();
	}
}

void SUnLive2DNorCreateDialog::MakeMotionPicker()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	FAssetPickerConfig AssetPickerConfig;
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 1
	AssetPickerConfig.Filter.ClassPaths.Add(UUnLive2D::StaticClass()->GetClassPathName());
#else
	AssetPickerConfig.Filter.ClassNames.Add(UUnLive2D::StaticClass()->GetFName());
#endif
	AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &SUnLive2DNorCreateDialog::OnUnLive2DSelected);
	AssetPickerConfig.OnShouldFilterAsset = FOnShouldFilterAsset::CreateSP(this, &SUnLive2DNorCreateDialog::FilterMotionBasedOnParentClass);
	AssetPickerConfig.bAllowNullSelection = true;
	AssetPickerConfig.InitialAssetViewType = EAssetViewType::Column;
	AssetPickerConfig.InitialAssetSelection = TargetUnLive2DAsset;

	UnLive2DContainer->ClearChildren();
	UnLive2DContainer->AddSlot()
	.AutoHeight()
	[
		SNew(STextBlock)
		.Text( NSLOCTEXT("UnLive2DMotionFactory","TargetUnLive2D", "Target Unlive2D:"))
		.ShadowOffset(FVector2D(1.0f, 1.0f))
	];

	UnLive2DContainer->AddSlot()
	[
		ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
	];
}

void SUnLive2DNorCreateDialog::OnUnLive2DSelected(const FAssetData& AssetData)
{
	TargetUnLive2DAsset = AssetData;
}

bool SUnLive2DNorCreateDialog::FilterMotionBasedOnParentClass(const FAssetData& AssetData)
{
	return false;
}
