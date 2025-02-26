#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "UObject/ObjectMacros.h"
#include "Widgets/Views/SListView.h"
#include "SUnLive2DParameterInfo.h"
#include "IUnLive2DParameterFilterText.h"

class SWindow;
class SComboButton;
class SUnLive2DParameterGroup;

class SUnLive2DAddParameterWidget : public SCompoundWidget , public IUnLive2DParameterFilterText
{

public:
	SLATE_BEGIN_ARGS(SUnLive2DAddParameterWidget)
	{}
	SLATE_END_ARGS()


public:

	void Construct(const FArguments& InArgs, TSharedPtr<SUnLive2DParameterGroup> ParameterGroup, TWeakObjectPtr<UUnLive2DRendererComponent> InUnLive2DComp);

	bool ConfigureProperties();

protected:
	// 访问器，以便我们的行可以获取用于高亮显示的filtertext
	virtual FText& GetFilterText() override { return FilterText; }


	void OnNameCommitted(const FText& NewName, ETextCommit::Type CommitType, TSharedPtr<FUnLive2DParameterInfo> Item) override;

protected:

	TSharedRef<SWidget> OnGetAddParameterList();

	FText GetAddParameterListDropDownText() const;

	void OnSelectionChanged(TSharedPtr<FUnLive2DParameterInfo> InItem, ESelectInfo::Type SelectInfo);
	// 参数集创建
	TSharedRef<ITableRow> GenerateUnLive2DParameterRow(TSharedPtr<FUnLive2DParameterInfo> InInfo, const TSharedRef<STableViewBase>& OwnerTable);

	void CloseDialog(bool bWasPicked = false);
protected:

	void CreateUnLive2DParameterList(const FString& SearchText = FString(), bool bInFullRefresh = false);

	void OnFilterTextChanged(const FText& SearchText);
	void OnFilterTextCommitted(const FText& SearchText, ETextCommit::Type CommitInfo);

protected:

	void OnUnLive2DParameterChanged(float NewParameter);
	TOptional<float> GetSpinBoxValue(FString SliderName) const;

	float GetParameterValue() const;

protected:
	FReply OkClicked();
	FReply CancelClicked();

protected:
	// 用户按ESC键取消
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

protected:

	TSharedRef<SWidget> OnGetShowOverrideTypeMenu();
	void HandleOverrideTypeChange(EUnLive2DExpressionBlendType::Type BlendType);
	FText GetOverrideTypeDropDownText() const;

public:

	TSharedPtr<FUnLive2DParameterInfo> SelectParameterInfo;

private:

	/** 输入NameFilterBox的当前文本 */
	FText FilterText;

	EUnLive2DExpressionBlendType::Type UnLive2DSelectBlendType;
private:

	/** 指向要求用户选择父类的窗口的指针 */
	TWeakPtr<SWindow> PickerWindow;

	// 点击确定
	bool bOkClicked;

	// 展示出来的参数
	TArray< TSharedPtr<FUnLive2DParameterInfo> > UnLive2DParameterList;

	// 所有的参数集
	TArray< TSharedPtr<FUnLive2DParameterInfo> > UnLive2DParameterByUID;


	TWeakObjectPtr<UUnLive2DRendererComponent> UnLive2DComp;

	// 参数集
	TSharedPtr<SListView< TSharedPtr<FUnLive2DParameterInfo> >>  UnLive2DParameterListView;

	// 参数集
	TArray<FUnLive2DParameterData> ParameterArr;

	TSharedPtr<SUnLive2DParameterGroup> ParameterGroup;

	TSharedPtr<SComboButton> AddParameterListComboButton;
};