#include "SUnLive2DParameterInfo.h"
#include "IUnLive2DParameterFilterText.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/SBoxPanel.h"
#include "Animation/UnLive2DExpression.h"
#include "IUnLive2DParameterEditorAsset.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "Animation/SmartName.h"
#include "CubismBpLib.h"


FName UnLive2DParameterNameLabel("Parameter Name");
FName UnLive2DParameterValueLabel("Parameter Value");

TSharedRef<FUnLive2DParameterInfo> FUnLive2DParameterInfo::Create(TWeakObjectPtr<UUnLive2DRendererComponent>& InEditableUnLive2D, const FUnLive2DDisplayParameterInfo& InSmartName, FUnLive2DParameterData& ParameterData)
{
	return MakeShareable(new FUnLive2DParameterInfo(InEditableUnLive2D, InSmartName, ParameterData));
}

TSharedRef<FUnLive2DParameterInfo> FUnLive2DParameterInfo::Create(TWeakObjectPtr<UUnLive2DRendererComponent>& InEditableUnLive2D, const FUnLive2DDisplayParameterInfo& InSmartName, FUnLive2DParameterData& ParameterData, EUnLive2DExpressionBlendType::Type InUnLive2DExpressionBlendType, TWeakObjectPtr<UUnLive2DAnimBase> InUnLive2DAnimBase)
{
	return MakeShareable(new FUnLive2DParameterInfo(InEditableUnLive2D, InSmartName, ParameterData, InUnLive2DExpressionBlendType, InUnLive2DAnimBase));
}

void SUnLive2DParameterListRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, IUnLive2DParameterFilterText* InParameterGroupPtr, EUnLive2DParameterAssetType::Type ParameterAssetType)
{
	Item = InArgs._Item;
	UnLive2DParameterGroupPtr = InParameterGroupPtr;
	UnLive2DParameterAssetType = ParameterAssetType;
	SMultiColumnTableRow< TSharedPtr<FUnLive2DParameterInfo> >::Construct(FSuperRowType::FArguments(), InOwnerTableView);
}

TSharedRef<SWidget> SUnLive2DParameterListRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (UnLive2DParameterGroupPtr == nullptr) return SNullWidget::NullWidget;

	if (ColumnName == UnLive2DParameterNameLabel)
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			.Padding(4)
			.VAlign(VAlign_Center)
			[
				SAssignNew(Item->EditableText, SInlineEditableTextBlock)
				.OnTextCommitted(this, &SUnLive2DParameterListRow::HandleOnTextCommitted, Item)
				.ColorAndOpacity(this, &SUnLive2DParameterListRow::GetItemTextColor)
				.IsSelected(this, &SUnLive2DParameterListRow::IsSelected)
				.Text(this, &SUnLive2DParameterListRow::GetItemName)
				.ToolTipText(this, &SUnLive2DParameterListRow::GetToolTipName)
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
				.Delta(0.1f)
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
	return FText::FromName(Item->ParameterData.DisplayName.IsNone() ? Item->ParameterData.ParameterID : Item->ParameterData.DisplayName);
}

FText SUnLive2DParameterListRow::GetFilterText() const
{
	if (UnLive2DParameterGroupPtr == nullptr) return FText::GetEmpty();

	return UnLive2DParameterGroupPtr->GetFilterText();

}

FText SUnLive2DParameterListRow::GetToolTipName() const
{
	return FText::FromName(Item->ParameterData.ParameterID);
}

void SUnLive2DParameterListRow::OnUnLive2DParameterChanged(float NewParameter)
{
	if (!Item->EditableUnLive2DComp.IsValid()) return;

	Item->ParameterData.ParameterValue = NewParameter;
	Item->bParameterModify = true;
	if (UnLive2DParameterAssetType == EUnLive2DParameterAssetType::UnLive2DExpression)
	{
		if (!Item->UnLive2DAnimBaseWeak.IsValid()) return;

		if (UUnLive2DExpression* Expression = Cast<UUnLive2DExpression>(Item->UnLive2DAnimBaseWeak.Get()))
		{
			Expression->SetAnimParamterValue(Item->EditableUnLive2DComp, Item->ParameterData.Index, NewParameter, UnLive2DExpressionBlendType);

		}

	}
	else if (UnLive2DParameterAssetType == EUnLive2DParameterAssetType::UnLive2D)
	{
		Item->EditableUnLive2DComp->SetModelParamterValue(Item->ParameterData.Index, NewParameter);
	}
}

void SUnLive2DParameterListRow::OnUnLive2DParameterValueCommitted(float NewParameter, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		if (!Item->EditableUnLive2DComp.IsValid()) return;

		Item->ParameterData.ParameterValue = NewParameter;
		Item->bParameterModify = true;

		if (UnLive2DParameterAssetType == EUnLive2DParameterAssetType::UnLive2DExpression)
		{
			if (!Item->UnLive2DAnimBaseWeak.IsValid()) return;

			if (UUnLive2DExpression* Expression = Cast<UUnLive2DExpression>(Item->UnLive2DAnimBaseWeak.Get()))
			{
				Expression->SetAnimParamterValue(Item->EditableUnLive2DComp, Item->ParameterData.Index, NewParameter, UnLive2DExpressionBlendType);

			}
		}
		else if (UnLive2DParameterAssetType == EUnLive2DParameterAssetType::UnLive2D)
		{
			Item->EditableUnLive2DComp->SetModelParamterValue(Item->ParameterData.Index, NewParameter);
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
	Item->bParameterModify = true;
	if (!Item->UnLive2DAnimBaseWeak.IsValid()) return;

	if (UUnLive2DExpression* Expression = Cast<UUnLive2DExpression>(Item->UnLive2DAnimBaseWeak.Get()))
	{
		Expression->SetAnimParamterBlendType(Item->EditableUnLive2DComp, Item->ParameterData.Index, Item->ParameterData.ParameterDefaultValue, UnLive2DExpressionBlendType);

	}
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

void SUnLive2DParameterListRow::HandleOnTextCommitted(const FText& NewName, ETextCommit::Type CommitType, TSharedPtr<FUnLive2DParameterInfo> InItem)
{
	if (UnLive2DParameterGroupPtr)
	{
		UnLive2DParameterGroupPtr->OnNameCommitted(NewName, CommitType, InItem);
	}
}