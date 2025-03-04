#pragma once

#include "CoreMinimal.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "UnLive2DRendererComponent.h"
#include "CubismBpLib.h"
#include "Animation/UnLive2DAnimBase.h"
#include "Widgets/Views/STableViewBase.h"
#include "IUnLive2DParameterEditorAsset.h"

class IUnLive2DParameterFilterText;

extern FName UnLive2DParameterNameLabel;
extern FName UnLive2DParameterValueLabel;

struct FUnLive2DDisplayParameterInfo
{
	int32 UID;
	// name 
	FName ParameterID;

	FName DisplayParameterName;
};

class FUnLive2DParameterInfo
{
public:
	FUnLive2DDisplayParameterInfo SmartName;
	FUnLive2DParameterData ParameterData;
	TWeakObjectPtr<UUnLive2DRendererComponent> EditableUnLive2DComp;
	TSharedPtr<SInlineEditableTextBlock> EditableText; // 列表中的可编辑文本框，用于从关联菜单中聚焦
	bool bShown;
	EUnLive2DExpressionBlendType::Type UnLive2DExpressionBlendType;
	TWeakObjectPtr<UUnLive2DAnimBase> UnLive2DAnimBaseWeak;

	bool bParameterModify;

	static TSharedRef<FUnLive2DParameterInfo> Create(TWeakObjectPtr<UUnLive2DRendererComponent>& InEditableUnLive2D, const FUnLive2DDisplayParameterInfo& InSmartName, FUnLive2DParameterData& ParameterData);

	static TSharedRef<FUnLive2DParameterInfo> Create(TWeakObjectPtr<UUnLive2DRendererComponent>& InEditableUnLive2D, const FUnLive2DDisplayParameterInfo& InSmartName, FUnLive2DParameterData& ParameterData, EUnLive2DExpressionBlendType::Type InUnLive2DExpressionBlendType, TWeakObjectPtr<UUnLive2DAnimBase> InUnLive2DAnimBase);

protected:
	FUnLive2DParameterInfo(TWeakObjectPtr<UUnLive2DRendererComponent>& EditableUnLive2DComp, const FUnLive2DDisplayParameterInfo& InSmartName, FUnLive2DParameterData& ParameterData)
		: SmartName(InSmartName)
		, ParameterData(MoveTemp(ParameterData))
		, EditableUnLive2DComp(EditableUnLive2DComp)
		, bShown(false)
		, UnLive2DAnimBaseWeak(nullptr)
		, bParameterModify(false)
	{}

	FUnLive2DParameterInfo(TWeakObjectPtr<UUnLive2DRendererComponent>& EditableUnLive2DComp, const FUnLive2DDisplayParameterInfo& InSmartName, FUnLive2DParameterData& ParameterData, EUnLive2DExpressionBlendType::Type InUnLive2DExpressionBlendType, TWeakObjectPtr<UUnLive2DAnimBase>& InUnLive2DAnimBase)
		: SmartName(InSmartName)
		, ParameterData(MoveTemp(ParameterData))
		, EditableUnLive2DComp(EditableUnLive2DComp)
		, bShown(false)
		, UnLive2DExpressionBlendType(InUnLive2DExpressionBlendType)
		, UnLive2DAnimBaseWeak(InUnLive2DAnimBase)
		, bParameterModify(false)
	{}

};

class SUnLive2DParameterListRow : public SMultiColumnTableRow< TSharedPtr<FUnLive2DParameterInfo> >
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DParameterListRow) {}
	SLATE_ARGUMENT(TSharedPtr<FUnLive2DParameterInfo>, Item)
		SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView, IUnLive2DParameterFilterText* InParameterGroupPtr, EUnLive2DParameterAssetType::Type ParameterAssetType);

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

	FText GetToolTipName() const;


	void OnUnLive2DParameterChanged(float NewParameter);
	void OnUnLive2DParameterValueCommitted(float NewParameter, ETextCommit::Type CommitType);


	TSharedRef<SWidget> OnGetShowOverrideTypeMenu();
	void HandleOverrideTypeChange(EUnLive2DExpressionBlendType::Type BlendType);
	FText GetOverrideTypeDropDownText() const;

	void HandleOnTextCommitted(const FText& NewName, ETextCommit::Type CommitType, TSharedPtr<FUnLive2DParameterInfo> Item);

private:
	TSharedPtr<FUnLive2DParameterInfo> Item;

	IUnLive2DParameterFilterText* UnLive2DParameterGroupPtr;

	EUnLive2DParameterAssetType::Type UnLive2DParameterAssetType;

	EUnLive2DExpressionBlendType::Type UnLive2DExpressionBlendType;
};