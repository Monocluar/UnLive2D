#include "SUnLive2DAssetFamilyShortcutBar.h"
#include "Widgets/Input/SCheckBox.h"
#include "IUnLive2DAssetFamily.h"
#include "Types/WidgetActiveTimerDelegate.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UnLive2DManagerModule.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"
#include "Misc/EngineVersionComparison.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "Styling/AppStyle.h"
#endif
#include "UnLive2DCubismCore.h"

#define LOCTEXT_NAMESPACE "SUnLive2DAssetFamilyShortcutBar"

namespace UnLive2DAssetShortcutConstants
{
	const int32 ThumbnailSize = 40;
	const int32 ThumbnailSizeSmall = 16;
}

class SUnLive2DAssetShortcut : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DAssetShortcut)
	{}

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, const TSharedRef<class FWorkflowCentricApplication>& InHostingApp, const TSharedRef<class IUnLive2DAssetFamily>& InAssetFamily, const FAssetData& InAssetData, const TSharedRef<FAssetThumbnailPool>& InThumbnailPool)
	{
		AssetData = InAssetData;
		AssetFamily = InAssetFamily;
		HostingApp = InHostingApp;
		ThumbnailPoolPtr = InThumbnailPool;
		bPackageDirty = false;

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		AssetRegistryModule.Get().OnFilesLoaded().AddSP(this, &SUnLive2DAssetShortcut::HandleFilesLoaded);
		AssetRegistryModule.Get().OnAssetAdded().AddSP(this, &SUnLive2DAssetShortcut::HandleAssetAdded);
		AssetRegistryModule.Get().OnAssetRemoved().AddSP(this, &SUnLive2DAssetShortcut::HandleAssetRemoved);
		AssetRegistryModule.Get().OnAssetRenamed().AddSP(this, &SUnLive2DAssetShortcut::HandleAssetRenamed);

		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OnAssetEditorRequestedOpen().AddSP(this, &SUnLive2DAssetShortcut::HandleAssetOpened);
		AssetFamily->GetOnAssetOpened().AddSP(this, &SUnLive2DAssetShortcut::HandleAssetOpened);

		AssetThumbnail = MakeShareable(new FAssetThumbnail(InAssetData, UnLive2DAssetShortcutConstants::ThumbnailSize, UnLive2DAssetShortcutConstants::ThumbnailSize, InThumbnailPool));
		AssetThumbnailSmall = MakeShareable(new FAssetThumbnail(InAssetData, UnLive2DAssetShortcutConstants::ThumbnailSizeSmall, UnLive2DAssetShortcutConstants::ThumbnailSizeSmall, InThumbnailPool));

		TArray<FAssetData> Assets;
		InAssetFamily->FindAssetsOfType(InAssetData.GetClass(), Assets);
		bMultipleAssetsExist = Assets.Num() > 1;
#if ENGINE_MAJOR_VERSION < 5
		AssetDirtyBrush = FEditorStyle::GetBrush("ContentBrowser.ContentDirty");
#else
		AssetDirtyBrush = FAppStyle::Get().GetBrush("Icons.DirtyBadge");
#endif

		{
			ChildSlot
			[
				CreateSwithUI()
			];}

		EnableToolTipForceField(true);
		DirtyStateTimerHandle = RegisterActiveTimer(1.0f / 10.0f, FWidgetActiveTimerDelegate::CreateSP(this, &SUnLive2DAssetShortcut::HandleRefreshDirtyState));
	}

	~SUnLive2DAssetShortcut()
	{
		if (FModuleManager::Get().IsModuleLoaded(TEXT("AssetRegistry")))
		{
			FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
			AssetRegistryModule.Get().OnFilesLoaded().RemoveAll(this);
			AssetRegistryModule.Get().OnAssetAdded().RemoveAll(this);
			AssetRegistryModule.Get().OnAssetRemoved().RemoveAll(this);
			AssetRegistryModule.Get().OnAssetRenamed().RemoveAll(this);
		}

		AssetFamily->GetOnAssetOpened().RemoveAll(this);
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OnAssetEditorRequestedOpen().RemoveAll(this);
		UnRegisterActiveTimer(DirtyStateTimerHandle.ToSharedRef());
	}

protected:

	TSharedRef<SWidget> CreateSwithUI()
	{
		return SAssignNew(CheckBox, SCheckBox)

#if ENGINE_MAJOR_VERSION < 5
				.Style(FUnLive2DStyle::Get(), "ToolBar.ToggleButton")
#else
				.Style(FUnLive2DStyle::Get(), "SegmentedCombo.Left")
#endif
				//.ForegroundColor(FSlateColor::UseForeground())
				.OnCheckStateChanged(this, &SUnLive2DAssetShortcut::HandleOpenAssetShortcut)
				.IsChecked(this, &SUnLive2DAssetShortcut::GetCheckState)
				.Visibility(this, &SUnLive2DAssetShortcut::GetButtonVisibility)
				.ToolTipText(this, &SUnLive2DAssetShortcut::GetButtonTooltip)
				.Padding(0.0f)
				[
#if ENGINE_MAJOR_VERSION < 5
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(SBorder)
						.Padding(4.0f)
						.BorderImage(FUnLive2DStyle::GetBrush("PropertyEditor.AssetThumbnailShadow"))
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							[
								SAssignNew(ThumbnailBox, SBox)
								.WidthOverride(UnLive2DAssetShortcutConstants::ThumbnailSize)
								.HeightOverride(UnLive2DAssetShortcutConstants::ThumbnailSize)
								.Visibility(this, &SUnLive2DAssetShortcut::GetThumbnailVisibility)
								[
									SNew(SOverlay)
									+SOverlay::Slot()
									[
										AssetThumbnail->MakeThumbnailWidget()
									]
									+SOverlay::Slot()
									.HAlign(HAlign_Left)
									.VAlign(VAlign_Bottom)
									[
										SNew(SImage)
										.Image(this, &SUnLive2DAssetShortcut::GetDirtyImage)
									]
								]
							]

							+SHorizontalBox::Slot()
							[
								SAssignNew(ThumbnailSmallBox, SBox)
								.WidthOverride(UnLive2DAssetShortcutConstants::ThumbnailSizeSmall)
								.HeightOverride(UnLive2DAssetShortcutConstants::ThumbnailSizeSmall)
								.Visibility(this, &SUnLive2DAssetShortcut::GetSmallThumbnailVisibility)
								[
									SNew(SOverlay)
									+SOverlay::Slot()
									[
										AssetThumbnailSmall->MakeThumbnailWidget()
									]
									+SOverlay::Slot()
									.HAlign(HAlign_Left)
									.VAlign(VAlign_Bottom)
									[
										SNew(SImage)
										.Image(this, &SUnLive2DAssetShortcut::GetDirtyImage)
									]
								]
							]
						]
					]

					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(4.0f)
						[
							SNew(STextBlock)
							.Text(this, &SUnLive2DAssetShortcut::GetAssetText)
							.TextStyle(FUnLive2DStyle::Get(), "Toolbar.Label")
							.ShadowOffset(FVector2D::UnitVector)
						]
					]
#else
					SNew(SOverlay)

					+ SOverlay::Slot()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.Padding(FMargin(16.f, 4.f))
					[
						SNew(SImage)
						.ColorAndOpacity(this, &SUnLive2DAssetShortcut::GetAssetTint)
						.Image(this, &SUnLive2DAssetShortcut::GetAssetIcon)
					]

					+ SOverlay::Slot()
					.VAlign(VAlign_Bottom)
					.HAlign(HAlign_Right)
					.Padding(FMargin(2.f, 2.f))
					[
						SNew(SImage)
						.ColorAndOpacity(FSlateColor::UseForeground())
						.Image(this, &SUnLive2DAssetShortcut::GetDirtyImage)
					]

#endif
				];
	}

protected:
	void HandleOpenAssetShortcut(ECheckBoxState InState)
	{
		if(AssetData.IsValid())
		{
			if (UObject* AssetObject = AssetData.GetAsset())
			{
				TArray<UObject*> Assets;
				Assets.Add(AssetObject);
				GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAssets(Assets);
			}
			else
			{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 1
				UE_LOG(LogUnLive2DManagerModule, Error, TEXT("Asset cannot be opened: %s"), *AssetData.GetSoftObjectPath().ToString());
#else
				UE_LOG(LogUnLive2DManagerModule, Error, TEXT("Asset cannot be opened: %s"), *AssetData.ObjectPath.ToString());
#endif
			}
		}
	}

	void RegenerateThumbnail()
	{
		if(AssetData.IsValid())
		{
			AssetThumbnail = MakeShareable(new FAssetThumbnail(AssetData, UnLive2DAssetShortcutConstants::ThumbnailSize, UnLive2DAssetShortcutConstants::ThumbnailSize, ThumbnailPoolPtr.Pin()));
			AssetThumbnailSmall = MakeShareable(new FAssetThumbnail(AssetData, UnLive2DAssetShortcutConstants::ThumbnailSizeSmall, UnLive2DAssetShortcutConstants::ThumbnailSizeSmall, ThumbnailPoolPtr.Pin()));

			if (ThumbnailBox)
			{
				ThumbnailBox->SetContent(AssetThumbnail->MakeThumbnailWidget());
			}

			if (ThumbnailSmallBox)
			{
				ThumbnailSmallBox->SetContent(AssetThumbnailSmall->MakeThumbnailWidget());
			}
		}
	}


	void HandleFilesLoaded()
	{
		TArray<FAssetData> Assets;
		AssetFamily->FindAssetsOfType(AssetData.GetClass(), Assets);
		bMultipleAssetsExist = Assets.Num() > 1;
	}

	void HandleAssetRemoved(const FAssetData& InAssetData)
	{
		if (AssetFamily->IsAssetCompatible(InAssetData))
		{
			TArray<FAssetData> Assets;
			AssetFamily->FindAssetsOfType(AssetData.GetClass(), Assets);
			bMultipleAssetsExist = Assets.Num() > 1;
		}
	}

	void HandleAssetRenamed(const FAssetData& InAssetData, const FString& InOldObjectPath)
	{
		if (AssetFamily->IsAssetCompatible(InAssetData))
		{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 1
			if (InOldObjectPath == AssetData.GetSoftObjectPath().ToString())
#else
			if (InOldObjectPath == AssetData.ObjectPath.ToString())
#endif
			{
				AssetData = InAssetData;

				RegenerateThumbnail();
			}
		}
	}

	void HandleAssetAdded(const FAssetData& InAssetData)
	{
		if (AssetFamily->IsAssetCompatible(InAssetData))
		{
			TArray<FAssetData> Assets;
			AssetFamily->FindAssetsOfType(AssetData.GetClass(), Assets);
			bMultipleAssetsExist = Assets.Num() > 1;
		}
	}

	void HandleAssetOpened(UObject* InAsset)
	{
		RefreshAsset();
	}

	FSlateColor GetAssetTint() const
	{
		if (GetCheckState() == ECheckBoxState::Checked)
		{
			return FSlateColor::UseForeground();
		}

		return AssetFamily->GetAssetTypeDisplayTint(AssetData.GetClass());
	}

	const FSlateBrush* GetAssetIcon() const
	{
		return AssetFamily->GetAssetTypeDisplayIcon(AssetData.GetClass());	
	}

	void RefreshAsset()
	{
		if(HostingApp.IsValid())
		{
			// if this is the asset being edited by our hosting asset editor, don't switch it
			bool bAssetBeingEdited = false;
			const TArray<UObject*>* Objects = HostingApp.Pin()->GetObjectsCurrentlyBeingEdited();
			if (Objects != nullptr)
			{
				for (UObject* Object : *Objects)
				{
					if (FAssetData(Object) == AssetData)
					{
						bAssetBeingEdited = true;
						break;
					}
				}
			}

			// switch to new asset if needed
			FAssetData NewAssetData = AssetFamily->FindAssetOfType(AssetData.GetClass());
			if (!bAssetBeingEdited && NewAssetData.IsValid() && NewAssetData != AssetData)
			{
				AssetData = NewAssetData;

				RegenerateThumbnail();
			}
		}
	}

protected:
	FText GetAssetText() const
	{
		return AssetFamily->GetAssetTypeDisplayName(AssetData.GetClass());
	}

	ECheckBoxState GetCheckState() const
	{
		if(HostingApp.IsValid())
		{
			const TArray<UObject*>* Objects = HostingApp.Pin()->GetObjectsCurrentlyBeingEdited();
			if (Objects != nullptr)
			{
				for (UObject* Object : *Objects)
				{
					if (FAssetData(Object) == AssetData)
					{
						return ECheckBoxState::Checked;
					}
				}
			}
		}
		return ECheckBoxState::Unchecked;
	}

	FSlateColor GetAssetTextColor() const
	{
		static const FName InvertedForeground("InvertedForeground");
#if ENGINE_MAJOR_VERSION >= 5
		return GetCheckState() == ECheckBoxState::Checked || CheckBox->IsHovered() ? FAppStyle::GetSlateColor(InvertedForeground) : FSlateColor::UseForeground();
#else
		return GetCheckState() == ECheckBoxState::Checked || CheckBox->IsHovered() ? FEditorStyle::GetSlateColor(InvertedForeground) : FSlateColor::UseForeground();
#endif
	}

	EVisibility GetButtonVisibility() const
	{
		return AssetData.IsValid() && !bMultipleAssetsExist ? EVisibility::Visible : EVisibility::Collapsed;
	}

	EVisibility GetComboVisibility() const
	{
		return bMultipleAssetsExist && AssetData.IsValid() ? EVisibility::Visible : EVisibility::Collapsed;
	}

	FText GetButtonTooltip() const
	{
		return FText::Format(LOCTEXT("AssetTooltipFormat", "{0}\n{1}"), FText::FromName(AssetData.AssetName), FText::FromString(AssetData.GetFullName()));
	}

	EVisibility GetThumbnailVisibility() const
	{
		return FMultiBoxSettings::UseSmallToolBarIcons.Get() ? EVisibility::Collapsed : EVisibility::Visible;
	}

	EVisibility GetSmallThumbnailVisibility() const
	{
		return FMultiBoxSettings::UseSmallToolBarIcons.Get() ? EVisibility::Visible : EVisibility::Collapsed;
	}
	
	const FSlateBrush* GetDirtyImage() const
	{
		return bPackageDirty ? AssetDirtyBrush : nullptr;
	}

	EActiveTimerReturnType HandleRefreshDirtyState(double InCurrentTime, float InDeltaTime)
	{
		if (AssetData.IsAssetLoaded())
		{
			if (!AssetPackage.IsValid())
			{
				AssetPackage = AssetData.GetPackage();
			}

			if (AssetPackage.IsValid())
			{
				bPackageDirty = AssetPackage->IsDirty();
			}
		}

		return EActiveTimerReturnType::Continue;
	}

private:
	
	/** 此小部件的当前资产数据 */
	FAssetData AssetData;

	/** 缓存对象的包以检查状态 */
	TWeakObjectPtr<UPackage> AssetPackage;

	/** 用于更新状态的计时器句柄 */
	TSharedPtr<class FActiveTimerHandle> DirtyStateTimerHandle;

	/** 与我们合作的资产组*/
	TSharedPtr<class IUnLive2DAssetFamily> AssetFamily;

	/** 我们的资产缩略图 */
	TSharedPtr<FAssetThumbnail> AssetThumbnail;
	TSharedPtr<FAssetThumbnail> AssetThumbnailSmall;

	/** 缩略图小部件容器 */
	TSharedPtr<SBox> ThumbnailBox;
	TSharedPtr<SBox> ThumbnailSmallBox;

	/** 我们嵌入的资产编辑器 */
	TWeakPtr<class FWorkflowCentricApplication> HostingApp;

	/** 缩略图池 */
	TWeakPtr<FAssetThumbnailPool> ThumbnailPoolPtr;
	
	/** Check box */
	TSharedPtr<SCheckBox> CheckBox;

	/** 缓存的纹理 */
	const FSlateBrush* AssetDirtyBrush;

	/** 是否存在该资产类型的多个（>1） */
	bool bMultipleAssetsExist;

	/** 缓存包的状态 */
	bool bPackageDirty;
};

void SUnLive2DAssetFamilyShortcutBar::Construct(const FArguments& InArgs, const TSharedRef<class FWorkflowCentricApplication>& InHostingApp, const TSharedRef<class IUnLive2DAssetFamily>& InAssetFamily)
{
	ThumbnailPool = MakeShareable(new FAssetThumbnailPool(16, false));

	TSharedRef<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox);

	TArray<UClass*> AssetTypes;
	InAssetFamily->GetAssetTypes(AssetTypes);

	int32 AssetTypeIndex = 0;
	for (UClass* Class : AssetTypes)
	{
		FAssetData AssetData = InAssetFamily->FindAssetOfType(Class);
		HorizontalBox->AddSlot()
		.AutoWidth()
		.Padding(0.f, 0.f, AssetTypeIndex == AssetTypes.Num() - 1 ? 0.0f: 2.0f, 0.0f)
		[
			SNew(SUnLive2DAssetShortcut, InHostingApp, InAssetFamily, AssetData, ThumbnailPool.ToSharedRef())
		];

		AssetTypeIndex++;
	}

	ChildSlot
	[
		HorizontalBox
	];
}

#undef LOCTEXT_NAMESPACE