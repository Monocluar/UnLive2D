
#pragma once
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SWindow.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SButton.h"
#include "EditorStyleSet.h"
#include "UnLive2DCubismCore.h"

#define LOCTEXT_NAMESPACE "FUnLive2DAssetEditorModule"

template<class FactoryClass>
class SUnLive2DAssetCreateDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DAssetCreateDialog)
	{}

	SLATE_END_ARGS()

public:
	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs)
	{
		bOkClicked = false;

		ChildSlot
		[
			SNew(SBorder)
			.Visibility(EVisibility::Visible)
			.BorderImage(FUnLive2DStyle::GetBrush("Menu.Background"))
			[
				SNew(SBox)
				.Visibility(EVisibility::Visible)
				.WidthOverride(500.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					.Padding(0.0f, 10.0f, 0.0f, 0.0f)
					[
						SNew(SBorder)
						.BorderImage(FUnLive2DStyle::GetBrush("ToolPanel.GroupBorder"))
						.Content()
						[
							SAssignNew(UnLive2DContainer, SVerticalBox)
						]
					]

					// Ok/Cancel buttons
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Bottom)
					.Padding(8.f)
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
							.OnClicked(this, &SUnLive2DAssetCreateDialog::OkClicked)
							.Text(NSLOCTEXT("Factory", "CreateAnimBlueprintOk", "OK"))
						]
						+ SUniformGridPanel::Slot(1, 0)
						[
							SNew(SButton)
							.HAlign(HAlign_Center)
							.ContentPadding(FUnLive2DStyle::GetMargin("StandardDialog.ContentPadding"))
							.OnClicked(this, &SUnLive2DAssetCreateDialog::CancelClicked)
							.Text(NSLOCTEXT("Factory", "CreateAnimBlueprintCancel", "Cancel"))
						]
					]
				]
			]
		];

		MakeUnLive2DPicker();
	}

	// Sets properties for the supplied UnLive2DBlueprintFactory
	bool ConfigureProperties(TWeakObjectPtr<FactoryClass> InAnimBlueprintFactory, FText TitleName)
	{
		TFactory = InAnimBlueprintFactory;

		TSharedRef<SWindow> Window = SNew(SWindow)
			.Title(TitleName)
			.ClientSize(FVector2D(400, 700))
			.SupportsMinimize(false)
			.SupportsMaximize(false)
			[
				AsShared()
			];

		PickerWindow = Window;

		GEditor->EditorAddModalWindow(Window);
		TFactory.Reset();

		return bOkClicked;
	}

private:

	/** Handler for when a parent class is selected */
	void OnClassPicked(UClass* ChosenClass)
	{
		ParentClass = ChosenClass;
		MakeUnLive2DPicker();
	}


	/** Creates the combo menu for the target UnLive2D */
	void MakeUnLive2DPicker()
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

		FAssetPickerConfig AssetPickerConfig;
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 1
		AssetPickerConfig.Filter.ClassPaths.Add(UUnLive2D::StaticClass()->GetClassPathName());
#else
		AssetPickerConfig.Filter.ClassNames.Add(UUnLive2D::StaticClass()->GetFName());
#endif
		AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &SUnLive2DAssetCreateDialog::OnUnLive2DSelected);
		AssetPickerConfig.bAllowNullSelection = true;
		AssetPickerConfig.InitialAssetViewType = EAssetViewType::Column;
		AssetPickerConfig.InitialAssetSelection = TargetUnLive2D;

		UnLive2DContainer->ClearChildren();
		UnLive2DContainer->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("TargetUnLive2D", "UnLive2D数据资源:"))
			.ShadowOffset(FVector2D(1.0f, 1.0f))
		];

		UnLive2DContainer->AddSlot()
		[
			ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
		];
	}


	/** Handler for when a skeleton is selected */
	void OnUnLive2DSelected(const FAssetData& AssetData)
	{
		TargetUnLive2D = AssetData;
	}

	void CloseDialog(bool bWasPicked = false)
	{
		bOkClicked = bWasPicked;
		if (PickerWindow.IsValid())
		{
			PickerWindow.Pin()->RequestDestroyWindow();
		}
	}

	/** Handler for when ok is clicked */
	FReply OkClicked()
	{
		if (TFactory.IsValid())
		{
			TFactory->TargetUnLive2D = Cast<UUnLive2D>(TargetUnLive2D.GetAsset());
		}

		if ( !TargetUnLive2D.IsValid())
		{
			// if TargetSkeleton is not valid
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NeedValidUnLive2D", "必须指定有效的UnLive2D数据."));
			return FReply::Handled();
		}

		CloseDialog(true);

		return FReply::Handled();
	}

	/** Handler for when cancel is clicked */
	FReply CancelClicked()
	{
		CloseDialog();
		return FReply::Handled();
	}

	FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
	{
		if (InKeyEvent.GetKey() == EKeys::Escape)
		{
			CloseDialog();
			return FReply::Handled();
		}
		return SWidget::OnKeyDown(MyGeometry, InKeyEvent);
	}

private:
	/** The factory for which we are setting up properties */
	TWeakObjectPtr<FactoryClass> TFactory;

	/** A pointer to the window that is asking the user to select a parent class */
	TWeakPtr<SWindow> PickerWindow;

	/** The container for the target skeleton picker*/
	TSharedPtr<SVerticalBox> UnLive2DContainer;

	/** The selected class */
	TWeakObjectPtr<UClass> ParentClass;

	/** True if Ok was clicked */
	bool bOkClicked;

	/** The selected UnLive2D */
	FAssetData TargetUnLive2D;
};

#undef LOCTEXT_NAMESPACE