#include "SUnLive2DParameterGroup.h"
#include "UnLive2D.h"
#include "UnLive2DRendererComponent.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "IUnLive2DParameterEditorAsset.h"
#include "Animation/UnLive2DAnimBase.h"

#define LOCTEXT_NAMESPACE "UnLive2DAssetEditor"

static const FName UnLive2DParameterNameLabel("Parameter Name");
static const FName UnLive2DParameterValueLabel("Parameter Value");

void SUnLive2DParameterGroup::Construct(const FArguments& InArgs, TSharedPtr<IUnLive2DParameterEditorAsset> InUnLive2DEditor)
{
	UnLive2DEditorPtr = InUnLive2DEditor;

	bShowAllParameter = true;
	if (!UnLive2DEditorPtr.IsValid()) return;

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
			.OnGenerateRow(this, &SUnLive2DParameterGroup::GenerateUnLive2DParameterRow, UnLive2DEditorPtr.Pin()->GetUnLive2DParameterAssetType())
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

void SUnLive2DParameterGroup::Construct(const FArguments& InArgs, TSharedPtr<IUnLive2DParameterEditorAsset> InUnLive2DEditor, TWeakObjectPtr<UUnLive2DAnimBase> InUnLive2DAnimBase)
{
	UnLive2DAnimBaseWeak = InUnLive2DAnimBase;
	SUnLive2DParameterGroup::Construct(InArgs, InUnLive2DEditor);
}

void SUnLive2DParameterGroup::UpDataUnLive2DAnimBase(TWeakObjectPtr<UUnLive2DAnimBase> InUnLive2DAnimBase)
{
	UnLive2DAnimBaseWeak = InUnLive2DAnimBase;
	CreateUnLive2DParameterList(FilterText.ToString(), true);
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

TSharedRef<ITableRow> SUnLive2DParameterGroup::GenerateUnLive2DParameterRow(TSharedPtr<FUnLive2DParameterInfo> InInfo, const TSharedRef<STableViewBase>& OwnerTable, EUnLive2DParameterAssetType::Type ParameterAssetType)
{
	return SNew(SUnLive2DParameterListRow, OwnerTable, SharedThis(this), ParameterAssetType)
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

	EUnLive2DParameterAssetType::Type ParameterAssetType = UnLive2DEditorPtr.Pin()->GetUnLive2DParameterAssetType();
	if (ParameterAssetType == EUnLive2DParameterAssetType::UnLive2D && UnLive2DParameterByUID.Num() != ParameterArr.Num())
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
	else if (ParameterAssetType == EUnLive2DParameterAssetType::UnLive2DExpression && UnLive2DAnimBaseWeak.IsValid())
	{
		TArray<FUnLive2DParameterData_Expression> ExpressionParameterArr;
		if (!UnLive2DAnimBaseWeak->GetAnimParamterGroup(Comp, ExpressionParameterArr)) return;

		UnLive2DParameterByUID.SetNum(ExpressionParameterArr.Num());

		int32 UIDIndex = 0;
		for (FUnLive2DParameterData_Expression& Item : ExpressionParameterArr)
		{
			if (!UnLive2DParameterByUID[UIDIndex].IsValid())
			{
				FSmartName SmartName;
				SmartName.UID = Item.ParameterID;
				SmartName.DisplayName = Item.ParameterName;
				TSharedRef<FUnLive2DParameterInfo> NewInfo = FUnLive2DParameterInfo::Create(Comp, SmartName, Item, Item.BlendType, UnLive2DAnimBaseWeak);
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

TSharedRef<FUnLive2DParameterInfo> FUnLive2DParameterInfo::Create(TWeakObjectPtr<UUnLive2DRendererComponent>& InEditableUnLive2D, const FSmartName& InSmartName, FUnLive2DParameterData& ParameterData, EUnLive2DExpressionBlendType::Type InUnLive2DExpressionBlendType, TWeakObjectPtr<UUnLive2DAnimBase>& InUnLive2DAnimBase)
{
	return MakeShareable(new FUnLive2DParameterInfo(InEditableUnLive2D, InSmartName, ParameterData, InUnLive2DExpressionBlendType, InUnLive2DAnimBase));
}

void SUnLive2DParameterListRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, TWeakPtr<SUnLive2DParameterGroup> InParameterGroupPtr, EUnLive2DParameterAssetType::Type ParameterAssetType)
{
	Item = InArgs._Item;
	UnLive2DParameterGroupPtr = InParameterGroupPtr;
	UnLive2DParameterAssetType = ParameterAssetType;
	SMultiColumnTableRow< TSharedPtr<FUnLive2DParameterInfo> >::Construct(FSuperRowType::FArguments(), InOwnerTableView);
}


TSharedRef<SWidget> SUnLive2DParameterListRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (!UnLive2DParameterGroupPtr.IsValid()) return SNullWidget::NullWidget;

	if (ColumnName == UnLive2DParameterNameLabel)
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
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
		TSharedPtr<SVerticalBox> VerticalBox = SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 1.0f)
			.VAlign(VAlign_Fill)
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

		if (UnLive2DParameterAssetType == EUnLive2DParameterAssetType::UnLive2DExpression)
		{
			UnLive2DExpressionBlendType = Item->UnLive2DExpressionBlendType;

			VerticalBox->AddSlot()
			[
				SNew(SComboButton)
				.OnGetMenuContent(this, &SUnLive2DParameterListRow::OnGetShowOverrideTypeMenu)
				.ContentPadding(2)
				.ButtonContent()
				[
					SNew(STextBlock)
					.Text(this, &SUnLive2DParameterListRow::GetOverrideTypeDropDownText)
				]
			];
		}

		return VerticalBox.ToSharedRef();
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

	if (UnLive2DParameterAssetType == EUnLive2DParameterAssetType::UnLive2DExpression)
	{
		if (!Item->UnLive2DAnimBaseWeak.IsValid()) return;

		Item->UnLive2DAnimBaseWeak->SetAnimParamterValue(Item->ParameterData.ParameterName, NewParameter);

	}
	else if (UnLive2DParameterAssetType == EUnLive2DParameterAssetType::UnLive2D)
	{
		Item->EditableUnLive2DComp->SetModelParamterValue(Item->ParameterData.ParameterID, NewParameter);
	}
}

void SUnLive2DParameterListRow::OnUnLive2DParameterValueCommitted(float NewParameter, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		if (!Item->EditableUnLive2DComp.IsValid()) return;

		Item->ParameterData.ParameterValue = NewParameter;

		if (UnLive2DParameterAssetType == EUnLive2DParameterAssetType::UnLive2DExpression)
		{
			if (!Item->UnLive2DAnimBaseWeak.IsValid()) return;

			Item->UnLive2DAnimBaseWeak->SetAnimParamterValue(Item->ParameterData.ParameterName, NewParameter);
			
		}
		else if (UnLive2DParameterAssetType == EUnLive2DParameterAssetType::UnLive2D)
		{
			Item->EditableUnLive2DComp->SetModelParamterValue(Item->ParameterData.ParameterID, NewParameter);
		}
	}
}

TSharedRef<SWidget> SUnLive2DParameterListRow::OnGetShowOverrideTypeMenu()
{
	FMenuBuilder MenuBuilder(true, NULL);

	auto CreateAction = [&MenuBuilder, this](EUnLive2DExpressionBlendType::Type InType, FString Name)
	{
		FUIAction NoAction(FExecuteAction::CreateSP(this, &SUnLive2DParameterListRow::HandleOverrideTypeChange, InType));
		MenuBuilder.AddMenuEntry(FText::FromString(Name), FText(), FSlateIcon(), NoAction);
	};

	CreateAction(EUnLive2DExpressionBlendType::ExpressionBlendType_Add, TEXT("Add"));
	CreateAction(EUnLive2DExpressionBlendType::ExpressionBlendType_Multiply, TEXT("Multiply"));
	CreateAction(EUnLive2DExpressionBlendType::ExpressionBlendType_Overwrite, TEXT("Overwrite"));

	return MenuBuilder.MakeWidget();
}

void SUnLive2DParameterListRow::HandleOverrideTypeChange(EUnLive2DExpressionBlendType::Type BlendType)
{
	UnLive2DExpressionBlendType = BlendType;
}

FText SUnLive2DParameterListRow::GetOverrideTypeDropDownText() const
{
	FText DropDownText;
	switch (UnLive2DExpressionBlendType)
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