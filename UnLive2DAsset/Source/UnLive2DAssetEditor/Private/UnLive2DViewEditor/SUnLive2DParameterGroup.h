
#pragma once

#include "CoreMinimal.h"
#include "UnLive2DViewEditor.h"
#include "Widgets/SCompoundWidget.h"
#include "CubismBpLib.h"
#include "Widgets/Views/STableViewBase.h"

class UUnLive2DRendererComponent;
struct FUnLive2DParameterData;

class FUnLive2DParameterInfo
{
public:
	FSmartName SmartName;
	FUnLive2DParameterData ParameterData;
	TWeakObjectPtr<UUnLive2DRendererComponent> EditableUnLive2DComp;
	TSharedPtr<SInlineEditableTextBlock> EditableText; // �б��еĿɱ༭�ı������ڴӹ����˵��о۽�
	bool bShown;

	static TSharedRef<FUnLive2DParameterInfo> Create(TWeakObjectPtr<UUnLive2DRendererComponent>& InEditableUnLive2D, const FSmartName& InSmartName, FUnLive2DParameterData& ParameterData);

protected:
	FUnLive2DParameterInfo(TWeakObjectPtr<UUnLive2DRendererComponent>& EditableUnLive2DComp, const FSmartName& InSmartName, FUnLive2DParameterData& ParameterData)
		: SmartName(InSmartName)
		, ParameterData(MoveTemp(ParameterData))
		, EditableUnLive2DComp(EditableUnLive2DComp)
		, bShown(false)
	{}
};

typedef SListView< TSharedPtr<FUnLive2DParameterInfo> > SUnLive2DParameterType;

class SUnLive2DParameterGroup : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DParameterGroup) {}
	SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs, TSharedPtr<FUnLive2DViewEditor> InUnLive2DEditor);


protected:
	
	void OnFilterTextChanged(const FText& SearchText);
	void OnFilterTextCommitted(const FText& SearchText, ETextCommit::Type CommitInfo);

protected:
	// �Ҽ��˵�
	TSharedPtr<SWidget> OnGetContextMenuContent() const;
	// ѡ�����
	void OnSelectionChanged(TSharedPtr<FUnLive2DParameterInfo> InItem, ESelectInfo::Type SelectInfo);
	// ����������
	TSharedRef<ITableRow> GenerateUnLive2DParameterRow(TSharedPtr<FUnLive2DParameterInfo> InInfo, const TSharedRef<STableViewBase>& OwnerTable);

private:

	void CreateUnLive2DParameterList(const FString& SearchText = FString(), bool bInFullRefresh = false);

	bool InitParameterGroupData();

public:

	// ���������Ա����ǵ��п��Ի�ȡ���ڸ�����ʾ��filtertext
	FORCEINLINE FText& GetFilterText() { return FilterText; }

	virtual void OnNameCommitted(const FText& NewName, ETextCommit::Type CommitType, TSharedPtr<FUnLive2DParameterInfo> Item);

private:

	TWeakPtr<class FUnLive2DViewEditor> UnLive2DEditorPtr;

	// ������
	TArray<FUnLive2DParameterData> ParameterArr;

	// ������
	TSharedPtr<SSearchBox>	NameFilterBox;

	// չʾ�����Ĳ���
	TArray< TSharedPtr<FUnLive2DParameterInfo> > UnLive2DParameterList;

	// ���еĲ�����
	TArray< TSharedPtr<FUnLive2DParameterInfo> > UnLive2DParameterByUID;

	// ������
	TSharedPtr<SUnLive2DParameterType>  UnLive2DParameterListView;

	/** ����NameFilterBox�ĵ�ǰ�ı� */
	FText FilterText;

	// �Ƿ�չʾ���в�����
	bool bShowAllParameter;
};

class SUnLive2DParameterListRow : public SMultiColumnTableRow< TSharedPtr<FUnLive2DParameterInfo> >
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DParameterListRow) {}
		SLATE_ARGUMENT(TSharedPtr<FUnLive2DParameterInfo>, Item)
	SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView, TWeakPtr<SUnLive2DParameterGroup> InParameterGroupPtr);

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

private:
	TSharedPtr<FUnLive2DParameterInfo> Item;

	TWeakPtr<SUnLive2DParameterGroup> UnLive2DParameterGroupPtr;
};