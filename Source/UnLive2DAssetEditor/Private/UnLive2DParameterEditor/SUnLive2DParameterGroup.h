
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "CubismBpLib.h"
#include "Widgets/Views/STableViewBase.h"
#include "IUnLive2DParameterEditorAsset.h"
#include "IUnLive2DParameterFilterText.h"

class UUnLive2DRendererComponent;
struct FUnLive2DParameterData;
class UUnLive2DAnimBase;

class SUnLive2DParameterGroup : public SCompoundWidget, public IUnLive2DParameterFilterText
{
	typedef SListView< TSharedPtr<FUnLive2DParameterInfo> > SUnLive2DParameterType;
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

	FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

protected:

	FReply OnSaveUnLiveParameterData();
	FReply OnAddParameterUnLiveData();

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
	virtual FText& GetFilterText() override { return FilterText; }

	virtual void OnNameCommitted(const FText& NewName, ETextCommit::Type CommitType, TSharedPtr<FUnLive2DParameterInfo> Item) override;


	TArray<TSharedPtr<FUnLive2DParameterInfo>> GetAllUnLive2DParameterByUID() const;
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

	TArray<TSharedPtr<FUnLive2DParameterInfo>> UnLive2DAddNewParameterList;

	TArray<TSharedPtr<FUnLive2DParameterInfo>> UnLive2DRemoveNewParameterList;
};
