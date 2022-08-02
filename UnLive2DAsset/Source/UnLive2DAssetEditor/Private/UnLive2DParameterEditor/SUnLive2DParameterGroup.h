
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "CubismBpLib.h"
#include "Widgets/Views/STableViewBase.h"
#include "IUnLive2DParameterEditorAsset.h"

class UUnLive2DRendererComponent;
struct FUnLive2DParameterData;
class UUnLive2DAnimBase;

class FUnLive2DParameterInfo
{
public:
	FSmartName SmartName;
	FUnLive2DParameterData ParameterData;
	TWeakObjectPtr<UUnLive2DRendererComponent> EditableUnLive2DComp;
	TSharedPtr<SInlineEditableTextBlock> EditableText; // 列表中的可编辑文本框，用于从关联菜单中聚焦
	bool bShown;
	EUnLive2DExpressionBlendType::Type UnLive2DExpressionBlendType;
	TWeakObjectPtr<UUnLive2DAnimBase> UnLive2DAnimBaseWeak;

	static TSharedRef<FUnLive2DParameterInfo> Create(TWeakObjectPtr<UUnLive2DRendererComponent>& InEditableUnLive2D, const FSmartName& InSmartName, FUnLive2DParameterData& ParameterData);

	static TSharedRef<FUnLive2DParameterInfo> Create(TWeakObjectPtr<UUnLive2DRendererComponent>& InEditableUnLive2D, const FSmartName& InSmartName, FUnLive2DParameterData& ParameterData, EUnLive2DExpressionBlendType::Type InUnLive2DExpressionBlendType, TWeakObjectPtr<UUnLive2DAnimBase>& InUnLive2DAnimBase);

protected:
	FUnLive2DParameterInfo(TWeakObjectPtr<UUnLive2DRendererComponent>& EditableUnLive2DComp, const FSmartName& InSmartName, FUnLive2DParameterData& ParameterData)
		: SmartName(InSmartName)
		, ParameterData(MoveTemp(ParameterData))
		, EditableUnLive2DComp(EditableUnLive2DComp)
		, bShown(false)
		, UnLive2DAnimBaseWeak(nullptr)
	{}

	FUnLive2DParameterInfo(TWeakObjectPtr<UUnLive2DRendererComponent>& EditableUnLive2DComp, const FSmartName& InSmartName, FUnLive2DParameterData& ParameterData, EUnLive2DExpressionBlendType::Type InUnLive2DExpressionBlendType, TWeakObjectPtr<UUnLive2DAnimBase>& InUnLive2DAnimBase)
		: SmartName(InSmartName)
		, ParameterData(MoveTemp(ParameterData))
		, EditableUnLive2DComp(EditableUnLive2DComp)
		, bShown(false)
		, UnLive2DExpressionBlendType(InUnLive2DExpressionBlendType)
		, UnLive2DAnimBaseWeak(InUnLive2DAnimBase)
	{}

};

typedef SListView< TSharedPtr<FUnLive2DParameterInfo> > SUnLive2DParameterType;

class SUnLive2DParameterGroup : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DParameterGroup) {}
	SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs, TSharedPtr<IUnLive2DParameterEditorAsset> InUnLive2DEditor);


	void Construct(const FArguments& InArgs, TSharedPtr<IUnLive2DParameterEditorAsset> InUnLive2DEditor, TWeakObjectPtr<UUnLive2DAnimBase> InUnLive2DAnimBase);

public:
	
	void UpDataUnLive2DAnimBase(TWeakObjectPtr<UUnLive2DAnimBase> InUnLive2DAnimBase);

protected:
	
	void OnFilterTextChanged(const FText& SearchText);
	void OnFilterTextCommitted(const FText& SearchText, ETextCommit::Type CommitInfo);

protected:
	// 右键菜单
	TSharedPtr<SWidget> OnGetContextMenuContent() const;
	// 选择参数
	void OnSelectionChanged(TSharedPtr<FUnLive2DParameterInfo> InItem, ESelectInfo::Type SelectInfo);
	// 参数集创建
	TSharedRef<ITableRow> GenerateUnLive2DParameterRow(TSharedPtr<FUnLive2DParameterInfo> InInfo, const TSharedRef<STableViewBase>& OwnerTable, EUnLive2DParameterAssetType::Type ParameterAssetType);

private:

	void CreateUnLive2DParameterList(const FString& SearchText = FString(), bool bInFullRefresh = false);

	bool InitParameterGroupData();

public:

	// 访问器，以便我们的行可以获取用于高亮显示的filtertext
	FORCEINLINE FText& GetFilterText() { return FilterText; }

	virtual void OnNameCommitted(const FText& NewName, ETextCommit::Type CommitType, TSharedPtr<FUnLive2DParameterInfo> Item);

private:

	TWeakPtr<class IUnLive2DParameterEditorAsset> UnLive2DEditorPtr;

	// 参数集
	TArray<FUnLive2DParameterData> ParameterArr;

	// 搜索框
	TSharedPtr<SSearchBox>	NameFilterBox;

	// 展示出来的参数
	TArray< TSharedPtr<FUnLive2DParameterInfo> > UnLive2DParameterList;

	// 所有的参数集
	TArray< TSharedPtr<FUnLive2DParameterInfo> > UnLive2DParameterByUID;

	// 参数集
	TSharedPtr<SUnLive2DParameterType>  UnLive2DParameterListView;

	/** 输入NameFilterBox的当前文本 */
	FText FilterText;

	// 是否展示所有参数组
	bool bShowAllParameter;


protected:
	TWeakObjectPtr<UUnLive2DAnimBase> UnLive2DAnimBaseWeak;
};

class SUnLive2DParameterListRow : public SMultiColumnTableRow< TSharedPtr<FUnLive2DParameterInfo> >
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DParameterListRow) {}
		SLATE_ARGUMENT(TSharedPtr<FUnLive2DParameterInfo>, Item)
	SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView, TWeakPtr<SUnLive2DParameterGroup> InParameterGroupPtr, EUnLive2DParameterAssetType::Type ParameterAssetType);

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

protected:

	/** Returns the Parameter of this curve */
	float GetParameterValue() const;
	/** Return color for text of item */
	FSlateColor GetItemTextColor() const;
	/** Returns name of this Parameter */
	FText GetItemName() const;
	/** Get text we are filtering for */
	FText GetFilterText() const;


	void OnUnLive2DParameterChanged(float NewParameter);
	void OnUnLive2DParameterValueCommitted(float NewParameter, ETextCommit::Type CommitType);


	TSharedRef<SWidget> OnGetShowOverrideTypeMenu();
	void HandleOverrideTypeChange(EUnLive2DExpressionBlendType::Type BlendType);
	FText GetOverrideTypeDropDownText() const;

private:
	TSharedPtr<FUnLive2DParameterInfo> Item;

	TWeakPtr<SUnLive2DParameterGroup> UnLive2DParameterGroupPtr;

	EUnLive2DParameterAssetType::Type UnLive2DParameterAssetType;

	EUnLive2DExpressionBlendType::Type UnLive2DExpressionBlendType;
};