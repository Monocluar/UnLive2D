#include "SUnLive2DMotionCrateDialog.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "UnLive2DMotionFactory.h"
#include "UnLive2D.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "UnLive2DMotion.h"
#include "ContentBrowserDelegates.h"
#include "UnLive2DMotionImportUI.h"

#define LOCTEXT_NAMESPACE "UnLive2DMotionFactory"

void SUnLive2DMotionCrateDialog::Construct(const FArguments& InArgs)
{
	ImportUI = InArgs._ImportUI;

	bOkClicked = false;

	ChildSlot
	[
		SNew(SBorder)
		.Visibility(EVisibility::Visible)
		.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
		[
			
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.FillHeight(1)
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				.Content()
				[
					SAssignNew(UnLive2DContainer, SVerticalBox)
				]
			]

			+ SVerticalBox::Slot()
			.FillHeight(1)
			.Padding(0.0f, 10.0f, 0.0f, 0.0f)
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				.Content()
				[
					SAssignNew(UnLive2DMotionPames, SVerticalBox)
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(8)
			[
				SNew(SUniformGridPanel)
				.SlotPadding(FEditorStyle::GetMargin("StandardDialog.SlotPadding"))
				.MinDesiredSlotWidth(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
				.MinDesiredSlotHeight(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))
				+ SUniformGridPanel::Slot(0, 0)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
					.OnClicked(this, &SUnLive2DMotionCrateDialog::OkClicked)
					.Text(NSLOCTEXT("AnimBlueprintFactory", "CreateAnimBlueprintOk", "OK"))
				]
				+ SUniformGridPanel::Slot(1, 0)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
					.OnClicked(this, &SUnLive2DMotionCrateDialog::CancelClicked)
					.Text(NSLOCTEXT("AnimBlueprintFactory", "CreateAnimBlueprintCancel", "Cancel"))
				]
			]
		]
	];

	MakeMotionPicker();
	MakeMotionDetail();
}

bool SUnLive2DMotionCrateDialog::ConfigureProperties(TWeakObjectPtr<UUnLive2DMotionFactory> InUnLive2DMotionFactory)
{
	UnLive2DMotionFactory = InUnLive2DMotionFactory;

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("CreateUnLive2DMotionOptions", "Create UnLive2D Motion Blueprint"))
		.ClientSize(FVector2D(400, 500))
		.SupportsMinimize(false).SupportsMaximize(false)
		[
			AsShared()
		];

	PickerWindow = Window;

	GEditor->EditorAddModalWindow(Window);
	UnLive2DMotionFactory.Reset(); //回收

	return bOkClicked;
}

FReply SUnLive2DMotionCrateDialog::OkClicked()
{
	if (UnLive2DMotionFactory.IsValid())
	{
		UnLive2DMotionFactory->BlueprintType = BPTYPE_Normal;
		UnLive2DMotionFactory->TargetUnLive2D = Cast<UUnLive2D>(TargetUnLive2DAsset.GetAsset());
	}

	if (!TargetUnLive2DAsset.IsValid())
	{
		// if TargetSkeleton is not valid
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NeedValidUnLive2D", "没有该UnLive2D数据驱动资源"));
		return FReply::Handled();
	}

	if (ImportUI->MotionGroupType == EUnLive2DMotionGroup::None)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NeedValidMotionGroupType", "动作组类型错误"));
		return FReply::Handled();
	}

	CloseDialog(true);

	return FReply::Handled();
}

FReply SUnLive2DMotionCrateDialog::CancelClicked()
{
	CloseDialog(false);
	return FReply::Handled();
}

void SUnLive2DMotionCrateDialog::CloseDialog(bool bWasPicked/*=false*/)
{
	bOkClicked = bWasPicked;
	if (PickerWindow.IsValid())
	{
		PickerWindow.Pin()->RequestDestroyWindow();
	}
}

void SUnLive2DMotionCrateDialog::MakeMotionPicker()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	FAssetPickerConfig AssetPickerConfig;
	AssetPickerConfig.Filter.ClassNames.Add(UUnLive2D::StaticClass()->GetFName());
	AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &SUnLive2DMotionCrateDialog::OnSkeletonSelected);
	AssetPickerConfig.OnShouldFilterAsset = FOnShouldFilterAsset::CreateSP(this, &SUnLive2DMotionCrateDialog::FilterSkeletonBasedOnParentClass);
	AssetPickerConfig.bAllowNullSelection = true;
	AssetPickerConfig.InitialAssetViewType = EAssetViewType::Column;
	AssetPickerConfig.InitialAssetSelection = TargetUnLive2DAsset;

	UnLive2DContainer->ClearChildren();
	UnLive2DContainer->AddSlot()
	.AutoHeight()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("TargetUnLive2D", "Target Unlive2D:"))
		.ShadowOffset(FVector2D(1.0f, 1.0f))
	];

	UnLive2DContainer->AddSlot()
	[
		ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
	];
}

void SUnLive2DMotionCrateDialog::MakeMotionDetail()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<class IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	UnLive2DMotionPames->AddSlot()
		[
			DetailsView->AsShared()
		];

	DetailsView->SetObject(ImportUI);
}

void SUnLive2DMotionCrateDialog::OnSkeletonSelected(const FAssetData& AssetData)
{
	TargetUnLive2DAsset = AssetData;
}

bool SUnLive2DMotionCrateDialog::FilterSkeletonBasedOnParentClass(const FAssetData& AssetData)
{
	return false;
}

#undef LOCTEXT_NAMESPACE