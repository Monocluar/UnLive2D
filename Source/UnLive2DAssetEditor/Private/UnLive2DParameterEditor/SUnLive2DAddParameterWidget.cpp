#include "SUnLive2DAddParameterWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SSearchBox.h"
#include "SUnLive2DParameterGroup.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SSpinBox.h"
#include "UnLive2DCubismCore.h"


#define LOCTEXT_NAMESPACE "UnLive2DAssetEditor"

void SUnLive2DAddParameterWidget::OnNameCommitted(const FText& InNewName, ETextCommit::Type CommitType, TSharedPtr<FUnLive2DParameterInfo> InItem)
{
	FName NewName(*InNewName.ToString());
	if (NewName == InItem->SmartName.DisplayName) return;

	if (NewName != NAME_None) return;

	FFormatNamedArguments Args;
	Args.Add(TEXT("InvalidName"), FText::FromName(NewName));
	FNotificationInfo Info(FText::Format(LOCTEXT("UnLive2DParameterRenamed", "The name \"{InvalidName}\" is invalid or already used."), Args));

	Info.bUseLargeFont = false;
	Info.ExpireDuration = 5.0f;

	TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);
	if (Notification.IsValid())
	{
		Notification->SetCompletionState(SNotificationItem::CS_Fail);
	}
}

void SUnLive2DAddParameterWidget::Construct(const FArguments& InArgs, TSharedPtr<SUnLive2DParameterGroup> InParameterGroup, TWeakObjectPtr<UUnLive2DRendererComponent> InUnLive2DComp)
{

	if (!InUnLive2DComp.IsValid()) return;
	ParameterGroup = InParameterGroup;
	UnLive2DComp = InUnLive2DComp;

	ChildSlot
	[
		SNew(SBorder)
		.Visibility(EVisibility::Visible)
		.BorderImage(FUnLive2DStyle::GetBrush("Menu.Background"))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 2)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.Padding(2.0f, 4.0f)
				.AutoWidth()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("NewAddParameterName", "AddParameterName"))
				]

				+ SHorizontalBox::Slot()
				.Padding(2.0f, 2.0f)
				.FillWidth(1.0f)
				.HAlign(HAlign_Fill)
				[
					SAssignNew(AddParameterListComboButton, SComboButton)
					.OnGetMenuContent(this, &SUnLive2DAddParameterWidget::OnGetAddParameterList)
					.ContentPadding(2)
					.ButtonContent()
					[
						SNew(STextBlock)
						.Text(this, &SUnLive2DAddParameterWidget::GetAddParameterListDropDownText)
					]
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SVerticalBox)
				.IsEnabled_Lambda([this](){ return SelectParameterInfo.IsValid(); })

				+ SVerticalBox::Slot()
				.Padding(0, 2)
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.Padding(2.0f, 4.0f)
					.AutoWidth()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("NewAddParameterValue", "AddParameterValue"))
					]

					+ SHorizontalBox::Slot()
					.Padding(2.0f, 2.0f)
					.FillWidth(1.0f)
					.HAlign(HAlign_Fill)
					[
						SNew(SSpinBox<float>)
						.MinSliderValue(this, &SUnLive2DAddParameterWidget::GetSpinBoxValue, FString("MinSlider"))
						.MaxSliderValue(this, &SUnLive2DAddParameterWidget::GetSpinBoxValue, FString("MaxSlider"))
						.MinValue(this, &SUnLive2DAddParameterWidget::GetSpinBoxValue, FString("MinValue"))
						.MaxValue(this, &SUnLive2DAddParameterWidget::GetSpinBoxValue, FString("MaxValue"))
						.Delta(0.1f)
						.Value(this, &SUnLive2DAddParameterWidget::GetParameterValue)
						.OnValueChanged(this, &SUnLive2DAddParameterWidget::OnUnLive2DParameterChanged)
					]
				]
				+ SVerticalBox::Slot()
				.Padding(0, 2)
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.Padding(2.0f, 4.0f)
					.AutoWidth()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("NewAddParameterBlendType", "AddParameterBlendType"))
					]

					+ SHorizontalBox::Slot()
					.Padding(2.0f, 2.0f)
					.FillWidth(1.0f)
					.HAlign(HAlign_Fill)
					[
						SNew(SComboButton)
						.OnGetMenuContent(this, &SUnLive2DAddParameterWidget::OnGetShowOverrideTypeMenu)
						.ContentPadding(2)
						.ButtonContent()
						[
							SNew(STextBlock)
							.Text(this, &SUnLive2DAddParameterWidget::GetOverrideTypeDropDownText)
						]
					]
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
					.IsEnabled_Lambda([this]() { return SelectParameterInfo.IsValid(); })
					.OnClicked(this, &SUnLive2DAddParameterWidget::OkClicked)
					.Text(NSLOCTEXT("AnimBlueprintFactory", "CreateAnimBlueprintOk", "OK"))
				]
				+ SUniformGridPanel::Slot(1, 0)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.ContentPadding(FUnLive2DStyle::GetMargin("StandardDialog.ContentPadding"))
					.OnClicked(this, &SUnLive2DAddParameterWidget::CancelClicked)
					.Text(NSLOCTEXT("AnimBlueprintFactory", "CreateAnimBlueprintCancel", "Cancel"))
				]
			]
		]
	];


	InUnLive2DComp->GetModelParamterGroup(ParameterArr);
}

bool SUnLive2DAddParameterWidget::ConfigureProperties()
{

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("AddParameterWidget", "AddParameterWidget"))
		.ClientSize(FVector2D(400, 130))
		.SupportsMinimize(false)
		.SupportsMaximize(false)
		[
			AsShared()
		];

	PickerWindow = Window;

	GEditor->EditorAddModalWindow(Window);
	return bOkClicked;
}

TSharedRef<SWidget> SUnLive2DAddParameterWidget::OnGetAddParameterList()
{

	TSharedPtr<SWidget> AddParameterListWidget =
	SNew(SBox)
	.MaxDesiredHeight(400.0f)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(SHorizontalBox)
			// Filter entry
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				SNew(SSearchBox)
				.SelectAllTextWhenFocused(true)
				.OnTextChanged(this, &SUnLive2DAddParameterWidget::OnFilterTextChanged)
				.OnTextCommitted(this, &SUnLive2DAddParameterWidget::OnFilterTextCommitted)
			]
		]
		
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SAssignNew(UnLive2DParameterListView, SListView< TSharedPtr<FUnLive2DParameterInfo> >)
			.ListItemsSource(&UnLive2DParameterList)
			.ItemHeight(22.0f)
			.SelectionMode(ESelectionMode::Multi)
			.OnSelectionChanged(this, &SUnLive2DAddParameterWidget::OnSelectionChanged)
			.OnGenerateRow(this, &SUnLive2DAddParameterWidget::GenerateUnLive2DParameterRow)
			.HeaderRow
			(
				SNew(SHeaderRow)
				.Visibility(EVisibility::Collapsed)
				+ SHeaderRow::Column(UnLive2DParameterNameLabel)
				.FillWidth(1.f)
				.DefaultLabel(LOCTEXT("UnLive2DParameterName", "Name"))
			)
		]
	];

	CreateUnLive2DParameterList(FilterText.ToString(), true);

	return AddParameterListWidget.ToSharedRef();
}

FText SUnLive2DAddParameterWidget::GetAddParameterListDropDownText() const
{
	if (SelectParameterInfo.IsValid())
	{
		return FText::FromName(SelectParameterInfo->SmartName.DisplayName);
	}
	return FText();
}

void SUnLive2DAddParameterWidget::OnSelectionChanged(TSharedPtr<FUnLive2DParameterInfo> InItem, ESelectInfo::Type SelectInfo)
{
	SelectParameterInfo = InItem;

	if (AddParameterListComboButton.IsValid())
	{
		AddParameterListComboButton->SetIsOpen(false);
	}
}

TSharedRef<ITableRow> SUnLive2DAddParameterWidget::GenerateUnLive2DParameterRow(TSharedPtr<FUnLive2DParameterInfo> InInfo, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SUnLive2DParameterListRow, OwnerTable, this, EUnLive2DParameterAssetType::UnLive2DExpression)
		.Item(InInfo);
}

void SUnLive2DAddParameterWidget::CloseDialog(bool bWasPicked /*= false*/)
{
	bOkClicked = bWasPicked;
	if (PickerWindow.IsValid())
	{
		PickerWindow.Pin()->RequestDestroyWindow();
	}
}

void SUnLive2DAddParameterWidget::CreateUnLive2DParameterList(const FString& SearchText /*= FString()*/, bool bInFullRefresh /*= false*/)
{
	if (!UnLive2DComp.IsValid()) return;

	bool bDirty = bInFullRefresh;

	UnLive2DParameterList.Reset();

	if (bInFullRefresh)
	{
		UnLive2DParameterByUID.Reset();
	}

	const TArray<TSharedPtr<FUnLive2DParameterInfo>>& FilterUnLive2DParameter = ParameterGroup->GetAllUnLive2DParameterByUID();
	if (UnLive2DParameterByUID.Num() != (ParameterArr.Num() - FilterUnLive2DParameter.Num()))
	{
		UnLive2DParameterByUID.SetNum(ParameterArr.Num() - FilterUnLive2DParameter.Num());
		int32 UIDIndex = 0;

		for (FUnLive2DParameterData& Item : ParameterArr)
		{

			bool bHas = FilterUnLive2DParameter.ContainsByPredicate([SmartName = Item.ParameterName](const TSharedPtr<FUnLive2DParameterInfo>& A)
			{
				return SmartName == A->ParameterData.ParameterName;
			});

			if (bHas) continue;

			if (!UnLive2DParameterByUID[UIDIndex].IsValid())
			{
				FSmartName SmartName;
				SmartName.UID = Item.ParameterID;
				SmartName.DisplayName = Item.ParameterName;

				TSharedRef<FUnLive2DParameterInfo> NewInfo = FUnLive2DParameterInfo::Create(UnLive2DComp, SmartName, Item, EUnLive2DExpressionBlendType::ExpressionBlendType_Add, nullptr);

				UnLive2DParameterByUID[UIDIndex] = NewInfo;
			}
			UIDIndex++;
		}
	}


	// Get set of active parameter
	//TMap<FName, float> ActiveParameter;
	for (TSharedPtr<FUnLive2DParameterInfo>& Item : UnLive2DParameterByUID)
	{
		FSmartName SmartName = Item->SmartName;

		bool bAddToList = true;
		if (!FilterText.IsEmpty())
		{
			if (!SmartName.DisplayName.ToString().Contains(*FilterText.ToString()))
			{
				bAddToList = false;
			}
		}
		/*if (bAddToList)
		{
			bAddToList = ActiveParameter.Contains(SmartName.DisplayName);
		}*/

		if (Item->bShown != bAddToList)
		{
			Item->bShown = bAddToList;
			bDirty = true;
		}

		if (bAddToList)
		{
			UnLive2DParameterList.Add(Item);
		}
	}
	if (bDirty)
	{
		if (UnLive2DParameterListView.IsValid())
		{
			UnLive2DParameterListView->RequestListRefresh();
		}
	}
}

void SUnLive2DAddParameterWidget::OnFilterTextChanged(const FText& SearchText)
{
	FilterText = SearchText;

	CreateUnLive2DParameterList(FilterText.ToString(), false);
}

void SUnLive2DAddParameterWidget::OnFilterTextCommitted(const FText& SearchText, ETextCommit::Type CommitInfo)
{
	CreateUnLive2DParameterList(FilterText.ToString(), true);
}

void SUnLive2DAddParameterWidget::OnUnLive2DParameterChanged(float NewParameter)
{
	if (!SelectParameterInfo.IsValid()) return;

	SelectParameterInfo->ParameterData.ParameterValue = NewParameter;
}

TOptional<float> SUnLive2DAddParameterWidget::GetSpinBoxValue(FString SliderName) const
{
	if (!SelectParameterInfo.IsValid()) return 0.f;

	if (SliderName == FString("MinSlider"))
	{
		return SelectParameterInfo->ParameterData.ParameterMinValue;
	}
	else if (SliderName == FString("MaxSlider"))
	{
		return SelectParameterInfo->ParameterData.ParameterMaxValue;
	}
	else if (SliderName == FString("MinValue"))
	{
		return SelectParameterInfo->ParameterData.ParameterMinValue;
	}
	else if (SliderName == FString("MaxValue"))
	{
		return SelectParameterInfo->ParameterData.ParameterMaxValue;
	}
	return 0.f;
}

float SUnLive2DAddParameterWidget::GetParameterValue() const
{
	if (!SelectParameterInfo.IsValid()) return 0.f;
	return SelectParameterInfo->ParameterData.ParameterValue;
}

FReply SUnLive2DAddParameterWidget::OkClicked()
{
	CloseDialog(true);
	return FReply::Handled();
}

FReply SUnLive2DAddParameterWidget::CancelClicked()
{
	CloseDialog(false);
	return FReply::Handled();
}

FReply SUnLive2DAddParameterWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		return CancelClicked();
	}

	return FReply::Unhandled();
}

TSharedRef<SWidget> SUnLive2DAddParameterWidget::OnGetShowOverrideTypeMenu()
{
	FMenuBuilder MenuBuilder(true, NULL);

	auto CreateAction = [&MenuBuilder, this](EUnLive2DExpressionBlendType::Type InType, FString Name)
	{
		FUIAction NoAction(FExecuteAction::CreateSP(this, &SUnLive2DAddParameterWidget::HandleOverrideTypeChange, InType));
		MenuBuilder.AddMenuEntry(FText::FromString(Name), FText(), FSlateIcon(), NoAction);
	};

	CreateAction(EUnLive2DExpressionBlendType::ExpressionBlendType_Add, TEXT("Add"));
	CreateAction(EUnLive2DExpressionBlendType::ExpressionBlendType_Multiply, TEXT("Multiply"));
	CreateAction(EUnLive2DExpressionBlendType::ExpressionBlendType_Overwrite, TEXT("Overwrite"));

	return MenuBuilder.MakeWidget();
}

void SUnLive2DAddParameterWidget::HandleOverrideTypeChange(EUnLive2DExpressionBlendType::Type BlendType)
{
	UnLive2DSelectBlendType = BlendType;
}

FText SUnLive2DAddParameterWidget::GetOverrideTypeDropDownText() const
{
	FText DropDownText;
	switch (UnLive2DSelectBlendType)
	{
	case EUnLive2DExpressionBlendType::ExpressionBlendType_Add:
		DropDownText = FText::FromString(TEXT("Add"));
		break;
	case EUnLive2DExpressionBlendType::ExpressionBlendType_Multiply:
		DropDownText = FText::FromString(TEXT("Multiply"));
		break;
	case EUnLive2DExpressionBlendType::ExpressionBlendType_Overwrite:
		DropDownText = FText::FromString(TEXT("Overwrite"));
		break;
	}

	return DropDownText;
}

#undef LOCTEXT_NAMESPACE