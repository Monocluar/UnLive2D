
#include "UnLive2DOptionWindow.h"
#include "IDetailsView.h"
#include "IDocumentation.h"
#include "UnLive2DImportUI.h"
#include "UnLive2DCubismCore.h"

#define LOCTEXT_NAMESPACE "UnLive2DOptionWindow"

void SUnLive2DOptionWindow::Construct(const FArguments& InArgs)
{
	ImportUI = InArgs._ImportUI;
	WidgetWindow = InArgs._WidgetWindow;

	TSharedPtr<SBox> ImportTypeDisplay;
	TSharedPtr<SBox> InspectorBox;

	// <Slate排布
	{
	ChildSlot
	[
		SNew(SBox)
		.MaxDesiredHeight(InArgs._MaxWindowHeight)
		.MaxDesiredWidth(InArgs._MaxWindowWidth)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2.f)
			[
				SAssignNew(ImportTypeDisplay, SBox)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2.f)
			[
				SNew(SBorder)
				.Padding(FMargin(3.f))
				.BorderImage(FUnLive2DStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.Font(FUnLive2DStyle::GetFontStyle("CurveEd.LabelFont"))
						.Text(NSLOCTEXT("FBXOption", "Import_CurrentFileTitle", "Current Asset: "))
					]
					
					+ SHorizontalBox::Slot()
					.Padding(5, 0, 0, 0)
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Font(FUnLive2DStyle::GetFontStyle("CurveEd.InfoFont"))
						.Text(FText::FromName(InArgs._FullPath))
						.ToolTipText(FText::FromName(InArgs._FullPath))
					]
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2.f)
			[
				SAssignNew(InspectorBox, SBox)
				.MaxDesiredHeight(650.0f)
				.WidthOverride(400.0f)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.Padding(2)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.Padding(2.f)
				[
					IDocumentation::Get()->CreateAnchor(FString("https://docs.google.com/document/d/1c6VZuOIZi2U1Lph2zUM-kvgujyt9UZ6S1-8nPG_zTYA/edit?usp=sharing"))
				]

				+ SHorizontalBox::Slot()
				.Padding(2.f)
				.AutoWidth()
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.Text(NSLOCTEXT("FBXOption","FbxOptionWindow_Import", "Import"))
					.OnClicked(this, &SUnLive2DOptionWindow::OnImport)
				]

				+ SHorizontalBox::Slot()
				.Padding(2.f)
				.AutoWidth()
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.Text(NSLOCTEXT("FBXOption", "FbxOptionWindow_Cancel", "Cancel"))
					.OnClicked(this, &SUnLive2DOptionWindow::OnCancel)
				]
			]
		]
	];
	}

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	InspectorBox->SetContent(DetailsView->AsShared());

	ImportTypeDisplay->SetContent
	(
		SNew(SBorder)
		.Padding(FMargin(3.f))
		.BorderImage(FUnLive2DStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("UnLive2DOptionWindow_ReImportType", "Reimport Live2D Data"))
			]

			+ SHorizontalBox::Slot()
			[
				SNew(SBox)
				.HAlign(HAlign_Right)
				[
					SNew(SButton)
					.Text(NSLOCTEXT("FBXOption", "FbxOptionWindow_ResetOptions", "Reset to Default"))
					.OnClicked(this, &SUnLive2DOptionWindow::OnResetToDefaultClick)
				]
			]
		]
	);

	DetailsView->SetObject(ImportUI);
}

FReply SUnLive2DOptionWindow::OnResetToDefaultClick() const
{
	ImportUI->ResetToDefault();

	DetailsView->SetObject(ImportUI, true);
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE