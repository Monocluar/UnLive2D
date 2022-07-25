#include "SUnLive2DParameterGroup.h"
#include "UnLive2D.h"
#include "UnLive2DRendererComponent.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

#define LOCTEXT_NAMESPACE "UnLive2DAssetEditor"

static const FName UnLive2DParameterNameLabel("Parameter Name");
static const FName UnLive2DParameterValueLabel("Parameter Value");

void SUnLive2DParameterGroup::Construct(const FArguments& InArgs, TSharedPtr<FUnLive2DViewEditor> InUnLive2DEditor)
{
	UnLive2DEditorPtr = InUnLive2DEditor;

	bShowAllParameter = true;
	if (!UnLive2DEditorPtr.IsValid()) return;

	UUnLive2D* UnLive2D = UnLive2DEditorPtr.Pin()->GetUnLive2DBeingEdited();

	if (UnLive2D == nullptr) return;

	ChildSlot
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
				SAssignNew(NameFilterBox, SSearchBox)
				.SelectAllTextWhenFocused(true)
				.OnTextChanged(this, &SUnLive2DParameterGroup::OnFilterTextChanged)
				.OnTextCommitted(this, &SUnLive2DParameterGroup::OnFilterTextCommitted)
			]
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SAssignNew(UnLive2DParameterListView, SUnLive2DParameterType)
			.ListItemsSource(&UnLive2DParameterList)
			.OnGenerateRow(this, &SUnLive2DParameterGroup::GenerateUnLive2DParameterRow)
			.OnContextMenuOpening(this, &SUnLive2DParameterGroup::OnGetContextMenuContent)
			.ItemHeight(22.0f)
			.SelectionMode(ESelectionMode::Multi)
			.OnSelectionChanged(this, &SUnLive2DParameterGroup::OnSelectionChanged)
			.HeaderRow
			(
				SNew(SHeaderRow)
				+ SHeaderRow::Column(UnLive2DParameterNameLabel)
				.FillWidth(1.f)
				.DefaultLabel(LOCTEXT("UnLive2DParameterName", "Name"))

				+ SHeaderRow::Column(UnLive2DParameterValueLabel)
				.FillWidth(0.5f)
				.DefaultLabel(LOCTEXT("UnLive2DParameterValue", "Value"))
			)
		]
	];

	if (InitParameterGroupData())
	{
		CreateUnLive2DParameterList(FilterText.ToString(), true);
	}
}

void SUnLive2DParameterGroup::OnFilterTextChanged(const FText& SearchText)
{
	FilterText = SearchText;

	CreateUnLive2DParameterList(FilterText.ToString(), false);
}

void SUnLive2DParameterGroup::OnFilterTextCommitted(const FText& SearchText, ETextCommit::Type CommitInfo)
{
	CreateUnLive2DParameterList(FilterText.ToString(), true);
}

TSharedPtr<SWidget> SUnLive2DParameterGroup::OnGetContextMenuContent() const
{
	FMenuBuilder MenuBuilder(true, NULL);
	MenuBuilder.BeginSection("UnLive2DParameterGroup", LOCTEXT("UnLive2DParameterGroup", "Parameter Group"));
	//MenuBuilder.AddMenuEntry(FGenericCommands::Get().Rename, NAME_None, LOCTEXT("RenameSmartNameLabel", "Rename Curve"), LOCTEXT("RenameSmartNameToolTip", "Rename the selected curve"));
	MenuBuilder.EndSection();
	return MenuBuilder.MakeWidget();

}

void SUnLive2DParameterGroup::OnSelectionChanged(TSharedPtr<FUnLive2DParameterInfo> InItem, ESelectInfo::Type SelectInfo)
{

}

TSharedRef<ITableRow> SUnLive2DParameterGroup::GenerateUnLive2DParameterRow(TSharedPtr<FUnLive2DParameterInfo> InInfo, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SUnLive2DParameterListRow, OwnerTable, SharedThis(this))
		.Item(InInfo);
}

void SUnLive2DParameterGroup::CreateUnLive2DParameterList(const FString& SearchText /*= FString()*/, bool bInFullRefresh /*= false*/)
{
	bool bDirty = bInFullRefresh;
	if (!UnLive2DEditorPtr.IsValid()) return;

	UnLive2DParameterList.Reset();

	if (bInFullRefresh)
	{
		UnLive2DParameterByUID.Reset();
	}

#if WITH_EDITOR

	TWeakObjectPtr<UUnLive2DRendererComponent> Comp = UnLive2DEditorPtr.Pin()->GetUnLive2DRenderComponent();
	if (UnLive2DParameterByUID.Num() != ParameterArr.Num())
	{
		UnLive2DParameterByUID.SetNum(ParameterArr.Num());

		int32 UIDIndex = 0;
		for (FUnLive2DParameterData& Item : ParameterArr)
		{
			if (!UnLive2DParameterByUID[UIDIndex].IsValid())
			{
				FSmartName SmartName;
				SmartName.UID = Item.ParameterID;
				SmartName.DisplayName = Item.ParameterName;

				TSharedRef<FUnLive2DParameterInfo> NewInfo = FUnLive2DParameterInfo::Create(Comp, SmartName, Item);

				UnLive2DParameterByUID[UIDIndex] = NewInfo;
			}
			UIDIndex++;
		}
	}

	// Get set of active parameter
	TMap<FName, float> ActiveParameter;
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
		if (bAddToList && !bShowAllParameter)
		{
			bAddToList = ActiveParameter.Contains(SmartName.DisplayName);
		}

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
		// Sort final list
		/*UnLive2DParameterList.Sort([](const TSharedPtr<FUnLive2DParameterInfo>& A, const TSharedPtr<FUnLive2DParameterInfo>& B) 
		{ return (A.Get()->SmartName.DisplayName.Compare(B.Get()->SmartName.DisplayName) < 0); });*/

		UnLive2DParameterListView->RequestListRefresh();
	}
#endif
}

bool SUnLive2DParameterGroup::InitParameterGroupData()
{
	if (!UnLive2DEditorPtr.IsValid()) return false;
	TWeakObjectPtr<UUnLive2DRendererComponent> Comp = UnLive2DEditorPtr.Pin()->GetUnLive2DRenderComponent();
	if (!Comp.IsValid()) return false;
	return Comp->GetModelParamterGroup(ParameterArr);
}

void SUnLive2DParameterGroup::OnNameCommitted(const FText& InNewName, ETextCommit::Type CommitType, TSharedPtr<FUnLive2DParameterInfo> Item)
{
	FName NewName(*InNewName.ToString());
	if (NewName == Item->SmartName.DisplayName) return;

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

TSharedRef<FUnLive2DParameterInfo> FUnLive2DParameterInfo::Create(TWeakObjectPtr<UUnLive2DRendererComponent>& InEditableUnLive2D, const FSmartName& InSmartName, FUnLive2DParameterData& ParameterData)
{
	return MakeShareable(new FUnLive2DParameterInfo(InEditableUnLive2D, InSmartName, ParameterData));
}

void SUnLive2DParameterListRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, TWeakPtr<SUnLive2DParameterGroup> InParameterGroupPtr)
{
	Item = InArgs._Item;
	UnLive2DParameterGroupPtr = InParameterGroupPtr;
	SMultiColumnTableRow< TSharedPtr<FUnLive2DParameterInfo> >::Construct(FSuperRowType::FArguments(), InOwnerTableView);
}


TSharedRef<SWidget> SUnLive2DParameterListRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (!UnLive2DParameterGroupPtr.IsValid()) return SNullWidget::NullWidget;

	if (ColumnName == UnLive2DParameterNameLabel)
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(4)
			.VAlign(VAlign_Center)
			[
				SAssignNew(Item->EditableText, SInlineEditableTextBlock)
				.OnTextCommitted(UnLive2DParameterGroupPtr.Pin().Get(), &SUnLive2DParameterGroup::OnNameCommitted, Item)
				.ColorAndOpacity(this, &SUnLive2DParameterListRow::GetItemTextColor)
				.IsSelected(this, &SUnLive2DParameterListRow::IsSelected)
				.Text(this, &SUnLive2DParameterListRow::GetItemName)
				.HighlightText(this, &SUnLive2DParameterListRow::GetFilterText)
			];
	}
	else if (ColumnName == UnLive2DParameterValueLabel)
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SSpinBox<float>)
				.MinSliderValue(Item->ParameterData.ParameterMinValue)
				.MaxSliderValue(Item->ParameterData.ParameterMaxValue)
				.MinValue(Item->ParameterData.ParameterMinValue)
				.MaxValue(Item->ParameterData.ParameterMaxValue)
				.Value(this, &SUnLive2DParameterListRow::GetParameterValue)
				.OnValueChanged(this, &SUnLive2DParameterListRow::OnUnLive2DParameterChanged)
				.OnValueCommitted(this, &SUnLive2DParameterListRow::OnUnLive2DParameterValueCommitted)
			];
	}

	return SNullWidget::NullWidget;
}

float SUnLive2DParameterListRow::GetParameterValue() const
{
	return Item->ParameterData.ParameterValue;
}

FSlateColor SUnLive2DParameterListRow::GetItemTextColor() const
{
	if (IsSelected())
	{
		return FLinearColor(0, 0, 0);
	}

	return FLinearColor(1, 1, 1);
}

FText SUnLive2DParameterListRow::GetItemName() const
{
	return FText::FromName(Item->ParameterData.ParameterName);
}

FText SUnLive2DParameterListRow::GetFilterText() const
{
	if (!UnLive2DParameterGroupPtr.IsValid()) return FText::GetEmpty();

	return UnLive2DParameterGroupPtr.Pin()->GetFilterText();
}

void SUnLive2DParameterListRow::OnUnLive2DParameterChanged(float NewParameter)
{
	if (!Item->EditableUnLive2DComp.IsValid()) return;

	Item->ParameterData.ParameterValue = NewParameter;

	Item->EditableUnLive2DComp->SetModelParamterValue(Item->ParameterData.ParameterID, NewParameter);
}

void SUnLive2DParameterListRow::OnUnLive2DParameterValueCommitted(float NewParameter, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		if (!Item->EditableUnLive2DComp.IsValid()) return;

		Item->ParameterData.ParameterValue = NewParameter;

		Item->EditableUnLive2DComp->SetModelParamterValue(Item->ParameterData.ParameterID, NewParameter);
	}
}

#undef LOCTEXT_NAMESPACE