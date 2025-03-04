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
#include "Animation/UnLive2DExpression.h"
#include "SUnLive2DAddParameterWidget.h"

#define LOCTEXT_NAMESPACE "UnLive2DAssetEditor"

FReply SUnLive2DParameterGroup::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Delete)
	{
		if (!UnLive2DEditorPtr.IsValid() || !UnLive2DEditorPtr.Pin()->GetUnLive2DParameterAddParameterData()) return FReply::Unhandled();

		if (!UnLive2DParameterListView.IsValid()) return FReply::Unhandled();

		for (TSharedPtr<FUnLive2DParameterInfo>& Item : UnLive2DParameterListView->GetSelectedItems())
		{
			
			if (UnLive2DAddNewParameterList.Contains(Item))
			{
				UnLive2DAddNewParameterList.Remove(Item);
			}
			else
			{
				UnLive2DRemoveNewParameterList.Add(Item);
			}
		}
		CreateUnLive2DParameterList(FilterText.ToString(), false);
		UnLive2DParameterListView->RequestListRefresh();
		//OnSaveUnLiveParameterData();
	}

	return FReply::Unhandled();
}

void SUnLive2DParameterGroup::Construct(const FArguments& InArgs, TSharedPtr<IUnLive2DParameterEditorAsset> InUnLive2DEditor)
{
	UnLive2DEditorPtr = InUnLive2DEditor;

	bShowAllParameter = true;
	if (!UnLive2DEditorPtr.IsValid()) return;

	TSharedPtr<SHorizontalBox> HorizontalBoxPanel;

	ChildSlot
	[
		SNew(SVerticalBox)


		+ SVerticalBox::Slot()
		.HAlign(HAlign_Center)
		.AutoHeight()
		[
			SAssignNew(HorizontalBoxPanel, SHorizontalBox)
		]

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
		.FillHeight(1.f)
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

	if (UnLive2DEditorPtr.Pin()->GetUnLive2DParameterAddParameterData()) // 添加参数
	{
		HorizontalBoxPanel->AddSlot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		.Padding(5.f)
		[
			SNew(SButton)
			.OnClicked(this, &SUnLive2DParameterGroup::OnAddParameterUnLiveData)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("UnLive2DAddParameterData", "AddParameter"))
			]
		];
	}

	if (UnLive2DEditorPtr.Pin()->GetUnLive2DParameterHasSaveData()) //保存按钮
	{
		HorizontalBoxPanel->AddSlot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		.Padding(5.f)
		[
			SNew(SButton)
			.OnClicked(this, &SUnLive2DParameterGroup::OnSaveUnLiveParameterData)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("UnLive2DParameterSaverData", "SaverData"))
			]
		];
	}

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

FReply SUnLive2DParameterGroup::OnSaveUnLiveParameterData()
{
	if (!UnLive2DAnimBaseWeak.IsValid()) return FReply::Unhandled();

	if (UUnLive2DExpression* Expression = Cast<UUnLive2DExpression>(UnLive2DAnimBaseWeak.Get()))
	{
		for (TSharedPtr<FUnLive2DParameterInfo>& Parameter : UnLive2DParameterByUID)
		{
			if (UnLive2DRemoveNewParameterList.Contains(Parameter))
			{
				Expression->RemoveExpressionDataValue(Parameter->ParameterData.ParameterID);
			}
			else
			{
				if (!Parameter->bParameterModify) continue;
				Expression->SetExpressionDataValue(Parameter->ParameterData.ParameterID, Parameter->ParameterData.ParameterValue, Parameter->UnLive2DExpressionBlendType);
			}
			
		}

		for (TSharedPtr<FUnLive2DParameterInfo>& AddParameter : UnLive2DAddNewParameterList)
		{
			Expression->AddExpressionDataValue(AddParameter->ParameterData.ParameterID, AddParameter->ParameterData.ParameterValue, AddParameter->UnLive2DExpressionBlendType);
		}

		Expression->SaveExpressionData();

		UnLive2DParameterByUID.Append(UnLive2DAddNewParameterList);
		
		UnLive2DAddNewParameterList.Empty();
		UnLive2DRemoveNewParameterList.Empty();
	}
	return FReply::Handled();
}

FReply SUnLive2DParameterGroup::OnAddParameterUnLiveData()
{
	TSharedPtr<SUnLive2DAddParameterWidget> AddParameterWidget = SNew(SUnLive2DAddParameterWidget, SharedThis(this), UnLive2DEditorPtr.Pin()->GetUnLive2DRenderComponent());

	if (AddParameterWidget->ConfigureProperties())
	{
		bool bIsHasRemove = false;
		AddParameterWidget->SelectParameterInfo->UnLive2DAnimBaseWeak = UnLive2DAnimBaseWeak;
		UnLive2DRemoveNewParameterList.RemoveAllSwap([SelectParameterInfo = AddParameterWidget->SelectParameterInfo, &bIsHasRemove](TSharedPtr<FUnLive2DParameterInfo>& A)
		{
			if (A->SmartName.ParameterID == SelectParameterInfo->SmartName.ParameterID)
			{
				A->ParameterData = SelectParameterInfo->ParameterData;
				A->bParameterModify = true;
				A->UnLive2DAnimBaseWeak = SelectParameterInfo->UnLive2DAnimBaseWeak;
				A->UnLive2DExpressionBlendType = SelectParameterInfo->UnLive2DExpressionBlendType;
				bIsHasRemove = true;
				return true;
			}
			return false;
		});

		if (!bIsHasRemove)
		{
			UnLive2DAddNewParameterList.Add(AddParameterWidget->SelectParameterInfo);
		}

		CreateUnLive2DParameterList(FilterText.ToString(), false);
		UnLive2DParameterListView->RequestListRefresh();
	}

	return FReply::Handled();
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
	return SNew(SUnLive2DParameterListRow, OwnerTable, this, ParameterAssetType)
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
				FUnLive2DDisplayParameterInfo SmartName;
				SmartName.UID = Item.Index;
				SmartName.ParameterID = Item.ParameterID;
				SmartName.DisplayParameterName = Item.DisplayName;

				TSharedRef<FUnLive2DParameterInfo> NewInfo = FUnLive2DParameterInfo::Create(Comp, SmartName, Item);

				UnLive2DParameterByUID[UIDIndex] = NewInfo;
			}
			UIDIndex++;
		}
	}
	else if (ParameterAssetType == EUnLive2DParameterAssetType::UnLive2DExpression && UnLive2DAnimBaseWeak.IsValid() && !UnLive2DParameterByUID.IsValidIndex(0))
	{
		TArray<FUnLive2DParameterData_Expression> ExpressionParameterArr;
		if (!UnLive2DAnimBaseWeak->GetAnimParamterGroup(Comp, ExpressionParameterArr)) return;

		UnLive2DParameterByUID.SetNum(ExpressionParameterArr.Num());

		int32 UIDIndex = 0;
		for (FUnLive2DParameterData_Expression& Item : ExpressionParameterArr)
		{
			if (!UnLive2DParameterByUID[UIDIndex].IsValid())
			{
				FUnLive2DDisplayParameterInfo SmartName;
				SmartName.UID = Item.Index;
				SmartName.ParameterID = Item.ParameterID;
				SmartName.DisplayParameterName = Item.DisplayName;

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
		if (UnLive2DRemoveNewParameterList.Contains(Item)) continue;
		FUnLive2DDisplayParameterInfo SmartName = Item->SmartName;
		bool bAddToList = true;
		if (!FilterText.IsEmpty())
		{
			if (!SmartName.ParameterID.ToString().Contains(*FilterText.ToString()))
			{
				bAddToList = false;
			}
			if (!SmartName.DisplayParameterName.ToString().Contains(*FilterText.ToString()))
			{
				bAddToList = false;
			}
		}
		if (bAddToList && !bShowAllParameter)
		{
			bAddToList = ActiveParameter.Contains(SmartName.ParameterID);
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

	for (TSharedPtr<FUnLive2DParameterInfo>& Item : UnLive2DAddNewParameterList)
	{
		FUnLive2DDisplayParameterInfo SmartName = Item->SmartName;
		bool bAddToList = true;
		if (!FilterText.IsEmpty())
		{
			if (!SmartName.ParameterID.ToString().Contains(*FilterText.ToString()))
			{
				bAddToList = false;
			}
			if (!SmartName.DisplayParameterName.ToString().Contains(*FilterText.ToString()))
			{
				bAddToList = false;
			}

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
	if (NewName == Item->SmartName.ParameterID) return;

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


TArray< TSharedPtr<FUnLive2DParameterInfo> > SUnLive2DParameterGroup::GetAllUnLive2DParameterByUID() const
{
	TArray<TSharedPtr<FUnLive2DParameterInfo>> ParameterInfo;

	for (const TSharedPtr<FUnLive2DParameterInfo>& Item : UnLive2DParameterByUID)
	{
		if (UnLive2DRemoveNewParameterList.Contains(Item)) continue;

		ParameterInfo.Add(Item);
	}

	ParameterInfo.Append(UnLive2DAddNewParameterList);

	return ParameterInfo;
}

#undef LOCTEXT_NAMESPACE