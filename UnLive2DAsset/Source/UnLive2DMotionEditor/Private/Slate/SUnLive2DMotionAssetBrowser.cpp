#include "SUnLive2DMotionAssetBrowser.h"
#include "ContentBrowserModule.h"
#include "UnLive2DMotion.h"
#include "IContentBrowserSingleton.h"
#include "UnLive2DMotionViewEditor.h"

void SUnLive2DMotionAssetBrowser::Construct(const FArguments& InArgs,  TSharedPtr<class FUnLive2DMotionViewEditor> InUnLive2DMotionEditor)
{
	UnLive2DMotionEditor = InUnLive2DMotionEditor;
	SourceMotionPtr = InUnLive2DMotionEditor->GetUnLive2DMotionEdited();

	RebuildWidget(InUnLive2DMotionEditor->GetUnLive2DMotionEdited());
}

void SUnLive2DMotionAssetBrowser::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	TSharedPtr<FUnLive2DMotionViewEditor> MotionEditor = UnLive2DMotionEditor.Pin();
	if (MotionEditor.IsValid())
	{
		if (SourceMotionPtr.Get() != MotionEditor->GetUnLive2DMotionEdited())
		{
			SourceMotionPtr = MotionEditor->GetUnLive2DMotionEdited();
			RebuildWidget(MotionEditor->GetUnLive2DMotionEdited());

		}
	}
}

void SUnLive2DMotionAssetBrowser::SelectAsset(UObject* InAsset)
{
	FAssetData AssetData(InAsset);

	if (AssetData.IsValid())
	{
		TArray<FAssetData> AssetsToSelect;
		AssetsToSelect.Add(AssetData);

		SyncToAssetsDelegate.Execute(AssetsToSelect);
	}
}

void SUnLive2DMotionAssetBrowser::RebuildWidget(UUnLive2DMotion* UnLive2DMotion)
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	FAssetPickerConfig AssetPickerConfig;
	AssetPickerConfig.Filter.ClassNames.Add(UUnLive2DMotion::StaticClass()->GetFName());
	AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &SUnLive2DMotionAssetBrowser::OnMotionSelected);
	AssetPickerConfig.OnAssetDoubleClicked = FOnAssetDoubleClicked::CreateSP(this, &SUnLive2DMotionAssetBrowser::OnMotionDoubleClicked);
	AssetPickerConfig.OnShouldFilterAsset = FOnShouldFilterAsset::CreateSP(this, &SUnLive2DMotionAssetBrowser::FilterMotionBasedOnParentClass);
	AssetPickerConfig.bAllowNullSelection = true;
	AssetPickerConfig.InitialAssetViewType = EAssetViewType::Column;
	AssetPickerConfig.InitialAssetSelection = TargetUnLive2DMotionAsset;

	AssetPickerConfig.SyncToAssetsDelegates.Add(&SyncToAssetsDelegate);

	ChildSlot
		[
			ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
		];
}

void SUnLive2DMotionAssetBrowser::OnMotionSelected(const FAssetData& AssetData)
{
}

void SUnLive2DMotionAssetBrowser::OnMotionDoubleClicked(const FAssetData& AssetData)
{
	TargetUnLive2DMotionAsset = AssetData;
	TSharedPtr<FUnLive2DMotionViewEditor> MotionViewEditor = UnLive2DMotionEditor.Pin();

	if (MotionViewEditor.IsValid())
	{
		if (UUnLive2DMotion* NewMotionToEdit = Cast<UUnLive2DMotion>(AssetData.GetAsset()))
		{
			MotionViewEditor->SetUnLive2DMotionBeingEdited(NewMotionToEdit);
		}
	}
}

bool SUnLive2DMotionAssetBrowser::FilterMotionBasedOnParentClass(const FAssetData& AssetData)
{
	if (!AssetData.IsValid()) return true;

	UUnLive2DMotion* TargetUnLive2DMotion = Cast<UUnLive2DMotion>(AssetData.GetAsset());
	TSharedPtr<FUnLive2DMotionViewEditor> MotionViewEditor = UnLive2DMotionEditor.Pin();

	if (TargetUnLive2DMotion && MotionViewEditor.IsValid())
	{
		return !(TargetUnLive2DMotion->UnLive2D == MotionViewEditor->GetUnLive2DMotionEdited()->UnLive2D);
	}

	return true;
}

