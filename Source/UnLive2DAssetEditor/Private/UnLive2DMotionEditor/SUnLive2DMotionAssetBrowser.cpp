#include "SUnLive2DMotionAssetBrowser.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "UnLive2DMotionViewEditor.h"
#include "Animation/UnLive2DAnimBase.h"
#include "Animation/UnLive2DExpression.h"
#include "Animation/UnLive2DMotion.h"

void SUnLive2DAnimBaseAssetBrowser::Construct(const FArguments& InArgs,  TSharedPtr<class FUnLive2DAnimBaseViewEditor> InUnLive2DAnimBaseEditor)
{
	UnLive2DAnimBaseEditor = InUnLive2DAnimBaseEditor;
	SourceAnimBasePtr = InUnLive2DAnimBaseEditor->GetUnLive2DAnimBaseEdited();

	RebuildWidget(InUnLive2DAnimBaseEditor->GetUnLive2DAnimBaseEdited());
}

void SUnLive2DAnimBaseAssetBrowser::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	TSharedPtr<FUnLive2DAnimBaseViewEditor> AnimBaseEditor = UnLive2DAnimBaseEditor.Pin();
	if (AnimBaseEditor.IsValid())
	{
		if (SourceAnimBasePtr.Get() != AnimBaseEditor->GetUnLive2DAnimBaseEdited())
		{
			SourceAnimBasePtr = AnimBaseEditor->GetUnLive2DAnimBaseEdited();
			RebuildWidget(AnimBaseEditor->GetUnLive2DAnimBaseEdited());

		}
	}
}

void SUnLive2DAnimBaseAssetBrowser::SelectAsset(UObject* InAsset)
{
	FAssetData AssetData(InAsset);

	if (AssetData.IsValid())
	{
		TArray<FAssetData> AssetsToSelect;
		AssetsToSelect.Add(AssetData);

		SyncToAssetsDelegate.Execute(AssetsToSelect);
	}
}

void SUnLive2DAnimBaseAssetBrowser::RebuildWidget(UUnLive2DAnimBase* UnLive2DAnimBas)
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	FAssetPickerConfig AssetPickerConfig;
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 1
	AssetPickerConfig.Filter.ClassPaths.Add(UUnLive2DMotion::StaticClass()->GetClassPathName());
	AssetPickerConfig.Filter.ClassPaths.Add(UUnLive2DExpression::StaticClass()->GetClassPathName());
#else
	AssetPickerConfig.Filter.ClassNames.Add(UUnLive2DMotion::StaticClass()->GetFName());
	AssetPickerConfig.Filter.ClassNames.Add(UUnLive2DExpression::StaticClass()->GetFName());
#endif
	AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &SUnLive2DAnimBaseAssetBrowser::OnAnimBaseSelected);
	AssetPickerConfig.OnAssetDoubleClicked = FOnAssetDoubleClicked::CreateSP(this, &SUnLive2DAnimBaseAssetBrowser::OnAnimBaseDoubleClicked);
	AssetPickerConfig.OnShouldFilterAsset = FOnShouldFilterAsset::CreateSP(this, &SUnLive2DAnimBaseAssetBrowser::FilterAnimBaseBasedOnParentClass);
	AssetPickerConfig.bAllowNullSelection = true;
	AssetPickerConfig.InitialAssetViewType = EAssetViewType::Column;
	if (!TargetUnLive2DAnimBaseAsset.IsValid())
	{
		TargetUnLive2DAnimBaseAsset = FAssetData(UnLive2DAnimBas);
	}

	AssetPickerConfig.InitialAssetSelection = TargetUnLive2DAnimBaseAsset;

	AssetPickerConfig.SyncToAssetsDelegates.Add(&SyncToAssetsDelegate);

	ChildSlot
		[
			ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
		];
}

void SUnLive2DAnimBaseAssetBrowser::OnAnimBaseSelected(const FAssetData& AssetData)
{
}

void SUnLive2DAnimBaseAssetBrowser::OnAnimBaseDoubleClicked(const FAssetData& AssetData)
{
	TargetUnLive2DAnimBaseAsset = AssetData;
	TSharedPtr<FUnLive2DAnimBaseViewEditor> AnimBaseViewEditor = UnLive2DAnimBaseEditor.Pin();

	if (AnimBaseViewEditor.IsValid())
	{
		if (UUnLive2DAnimBase* NewAnimBaseToEdit = Cast<UUnLive2DAnimBase>(AssetData.GetAsset()))
		{
			AnimBaseViewEditor->SetUnLive2DAnimBeingEdited(NewAnimBaseToEdit);
		}
		
	}
}

bool SUnLive2DAnimBaseAssetBrowser::FilterAnimBaseBasedOnParentClass(const FAssetData& AssetData)
{
	if (!AssetData.IsValid()) return true;

	TSharedPtr<FUnLive2DAnimBaseViewEditor> AnimBaseViewEditor = UnLive2DAnimBaseEditor.Pin();

	if (!AnimBaseViewEditor.IsValid()) return true;

	UUnLive2DAnimBase* TargetUnLive2DAnimBase = Cast<UUnLive2DAnimBase>(AssetData.GetAsset());


	if (TargetUnLive2DAnimBase )
	{
		return !(TargetUnLive2DAnimBase->UnLive2D == AnimBaseViewEditor->GetUnLive2DAnimBaseEdited()->UnLive2D);
	}

	return true;
}

