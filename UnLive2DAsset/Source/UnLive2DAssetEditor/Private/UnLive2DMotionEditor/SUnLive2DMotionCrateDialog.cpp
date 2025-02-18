#include "SUnLive2DMotionCrateDialog.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "UnLive2DMotionFactory.h"
#include "UnLive2D.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Animation/UnLive2DMotion.h"
#include "ContentBrowserDelegates.h"
#include "UnLive2DMotionImportUI.h"
#include "UnLive2DExpressionFactory.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UnLive2DCubismCore.h"

#define LOCTEXT_NAMESPACE "UnLive2DMotionFactory"

template<typename AssetType>
static void FindAssets(const UUnLive2D* InUnLive2D, TArray<FAssetData>& OutAssetData)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FARFilter Filter;
	Filter.bRecursiveClasses = true;
	Filter.ClassNames.Add(AssetType::StaticClass()->GetFName());
	Filter.TagsAndValues.Add(TEXT("UnLive2D"), FAssetData(InUnLive2D).GetExportTextName());

	AssetRegistryModule.Get().GetAssets(Filter, OutAssetData);
}

void SUnLive2DMotionCrateDialog::Construct(const FArguments& InArgs)
{
	ImportUI = InArgs._ImportUI;

	bOkClicked = false;

	TSharedPtr<SBorder> MotionPamesBorder;

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
			.FillHeight(1)
			.Padding(0.0f, 10.0f, 0.0f, 0.0f)
			[
				SAssignNew(MotionPamesBorder, SBorder)
				.BorderImage(FUnLive2DStyle::GetBrush("ToolPanel.GroupBorder"))
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
				.SlotPadding(FUnLive2DStyle::GetMargin("StandardDialog.SlotPadding"))
				.MinDesiredSlotWidth(FUnLive2DStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
				.MinDesiredSlotHeight(FUnLive2DStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))
				+ SUniformGridPanel::Slot(0, 0)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.ContentPadding(FUnLive2DStyle::GetMargin("StandardDialog.ContentPadding"))
					.OnClicked(this, &SUnLive2DMotionCrateDialog::OkClicked)
					.Text(NSLOCTEXT("AnimBlueprintFactory", "CreateAnimBlueprintOk", "OK"))
				]
				+ SUniformGridPanel::Slot(1, 0)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.ContentPadding(FUnLive2DStyle::GetMargin("StandardDialog.ContentPadding"))
					.OnClicked(this, &SUnLive2DMotionCrateDialog::CancelClicked)
					.Text(NSLOCTEXT("AnimBlueprintFactory", "CreateAnimBlueprintCancel", "Cancel"))
				]
			]
		]
	];

	if (ImportUI == nullptr)
	{
		MotionPamesBorder->SetVisibility(EVisibility::Collapsed);
	}

	MakeMotionPicker();
	MakeMotionDetail();
}

bool SUnLive2DMotionCrateDialog::ConfigureProperties(TWeakObjectPtr<UFactory> InUnLive2DMotionFactory)
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
	MotionAssetArr.Empty();

	if (!UnLive2DMotionFactory.IsValid()) return FReply::Handled();

	if (UUnLive2DMotionFactory* MotionFactory = Cast<UUnLive2DMotionFactory>(UnLive2DMotionFactory.Get()))
	{
		MotionFactory->TargetUnLive2D = Cast<UUnLive2D>(TargetUnLive2DAsset.GetAsset());

		if ((MotionFactory->TargetUnLive2D != nullptr))
		{
			FindAssets<UUnLive2DMotion>(MotionFactory->TargetUnLive2D, MotionAssetArr);
		}
	}
	else if (UUnLive2DExpressionFactory* ExpressionFactory = Cast<UUnLive2DExpressionFactory>(UnLive2DMotionFactory.Get()))
	{
		ExpressionFactory->TargetUnLive2D = Cast<UUnLive2D>(TargetUnLive2DAsset.GetAsset());
		if ((ExpressionFactory->TargetUnLive2D != nullptr))
		{
			FindAssets<UUnLive2DExpressionFactory>(ExpressionFactory->TargetUnLive2D, MotionAssetArr);
		}
	}

	if (!TargetUnLive2DAsset.IsValid())
	{
		// if TargetSkeleton is not valid
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NeedValidUnLive2D", "没有该UnLive2D数据驱动资源"));
		return FReply::Handled();
	}

	if (ImportUI != nullptr)
	{
		if (ImportUI->MotionGroupType == EUnLive2DMotionGroup::None)
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NeedValidMotionGroupType", "动作组类型错误"));
			return FReply::Handled();
		}

		if (HasUnLive2DMotion(ImportUI->MotionCount, ImportUI->MotionGroupType))
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("HasNeedValidMotione", "已经重复了动作组"));
			return FReply::Handled();
		}

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
	AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &SUnLive2DMotionCrateDialog::OnUnLive2DSelected);
	AssetPickerConfig.OnShouldFilterAsset = FOnShouldFilterAsset::CreateSP(this, &SUnLive2DMotionCrateDialog::FilterMotionBasedOnParentClass);
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
	if (ImportUI == nullptr) return;

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

void SUnLive2DMotionCrateDialog::OnUnLive2DSelected(const FAssetData& AssetData)
{
	TargetUnLive2DAsset = AssetData;
}

bool SUnLive2DMotionCrateDialog::FilterMotionBasedOnParentClass(const FAssetData& AssetData)
{
	return false;
}

bool SUnLive2DMotionCrateDialog::HasUnLive2DMotion(int32 MotionCount, EUnLive2DMotionGroup MotionGroupType)
{
	for (FAssetData& Item : MotionAssetArr)
	{
		UUnLive2DMotion* Motion = Cast<UUnLive2DMotion>(Item.GetAsset());
		if (Motion)
		{
			const FUnLive2DMotionData* MotionData = Motion->GetMotionData();
			if (MotionData->MotionCount == MotionCount && MotionData->MotionGroupType == MotionGroupType )
			{
				return true;
			}
		}
	}

	return false;
}

#undef LOCTEXT_NAMESPACE