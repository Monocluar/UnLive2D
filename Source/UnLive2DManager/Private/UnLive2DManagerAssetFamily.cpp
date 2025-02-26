#include "UnLive2DManagerAssetFamily.h"
#include "UnLive2D.h"
#include "Animation/UnLive2DAnimBase.h"
#include "Animation/UnLive2DAnimBlueprint.h"
#include "AssetToolsModule.h"
#include "Misc/EngineVersionComparison.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UnLive2DCubismCore.h"

#define LOCTEXT_NAMESPACE "UnLive2DManagerAssetFamily"

template<typename AssetType>
static void FindAssets(const UUnLive2D* InUnLive2D, TArray<FAssetData>& OutAssetData, FName UnLive2DTag)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FARFilter Filter;
	Filter.bRecursiveClasses = true;
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 1
	Filter.ClassPaths.Add(AssetType::StaticClass()->GetClassPathName());
#else
	Filter.ClassNames.Add(AssetType::StaticClass()->GetFName());
#endif
	Filter.TagsAndValues.Add(UnLive2DTag, FAssetData(InUnLive2D).GetExportTextName());

	AssetRegistryModule.Get().GetAssets(Filter, OutAssetData);
}

FUnLive2DManagerAssetFamily::FUnLive2DManagerAssetFamily(const UObject* InFromObject)
	: UnLive2D(nullptr)
	, UnLive2DAnimBase(nullptr)
{
	if (InFromObject)
	{
		if (InFromObject->IsA<UUnLive2D>())
		{
			UnLive2D = CastChecked<UUnLive2D>(InFromObject);
		}
		else if (InFromObject->IsA<UUnLive2DAnimBase>())
		{
			UnLive2DAnimBase = CastChecked<UUnLive2DAnimBase>(InFromObject);
		}
		else if (InFromObject->IsA<UUnLive2DAnimBlueprint>())
		{
			UnLive2DAnimBlueprint = CastChecked<UUnLive2DAnimBlueprint>(InFromObject);
		}

		FindCounterpartAssets(InFromObject, UnLive2D);
	}
}

FSlateColor FUnLive2DManagerAssetFamily::GetAssetTypeDisplayTint(UClass* InAssetClass) const
{
	UClass* UseAssetClass = nullptr;
	if (InAssetClass == nullptr) return FSlateColor::UseForeground();

	if (InAssetClass->IsChildOf<UUnLive2D>())
	{
		UseAssetClass = UUnLive2D::StaticClass();
	}
	else if (InAssetClass->IsChildOf<UUnLive2DAnimBase>())
	{
		UseAssetClass = UUnLive2DAnimBase::StaticClass();
	}
	else if (InAssetClass->IsChildOf<UUnLive2DAnimBlueprint>())
	{
		UseAssetClass = UUnLive2DAnimBlueprint::StaticClass();
	}

	if (UseAssetClass == nullptr) return FSlateColor::UseForeground();

	static const FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

	TWeakPtr<IAssetTypeActions> AssetTypeActions = AssetToolsModule.Get().GetAssetTypeActionsForClass(UseAssetClass);
	if (AssetTypeActions.IsValid())
	{
		return AssetTypeActions.Pin()->GetTypeColor();
	}

	return FSlateColor::UseForeground();
}

const FSlateBrush* FUnLive2DManagerAssetFamily::GetAssetTypeDisplayIcon(UClass* InAssetClass) const
{

	if (InAssetClass)
	{
		if (InAssetClass->IsChildOf<UUnLive2D>())
		{
			return FUnLive2DStyle::Get().GetBrush("Persona.AssetClass.Skeleton");
		}
		else if (InAssetClass->IsChildOf<UUnLive2DAnimBase>())
		{
			return FUnLive2DStyle::Get().GetBrush("Persona.AssetClass.Animation");
		}
		else if (InAssetClass->IsChildOf<UUnLive2DAnimBlueprint>())
		{
			return FUnLive2DStyle::Get().GetBrush("Persona.AssetClass.Blueprint");
		}
	}

	return nullptr;
}

void FUnLive2DManagerAssetFamily::FindCounterpartAssets(const UObject* InAsset, TWeakObjectPtr<const UUnLive2D>& OutUnLive2D)
{
	const UUnLive2D* CounterpartUnLive2D = OutUnLive2D.Get();
	FindCounterpartAssets(InAsset, CounterpartUnLive2D);
	OutUnLive2D = CounterpartUnLive2D;
}

void FUnLive2DManagerAssetFamily::FindCounterpartAssets(const UObject* InAsset, const UUnLive2D*& OutUnLive2D)
{
	if (InAsset->IsA<UUnLive2D>())
	{
		OutUnLive2D = CastChecked<UUnLive2D>(InAsset);
	}
	else if (InAsset->IsA<UUnLive2DAnimBase>())
	{
		const UUnLive2DAnimBase* MotionAsset = CastChecked<const UUnLive2DAnimBase>(InAsset);
		OutUnLive2D = MotionAsset->UnLive2D;
	}
	else if (InAsset->IsA<UUnLive2DAnimBlueprint>())
	{
		const UUnLive2DAnimBlueprint* UnLive2DAnimBlueprintAsset = CastChecked<const UUnLive2DAnimBlueprint>(InAsset);
		OutUnLive2D = UnLive2DAnimBlueprintAsset->TargetUnLive2D;
	}
}

void FUnLive2DManagerAssetFamily::GetAssetTypes(TArray<UClass*>& OutAssetTypes) const
{
	OutAssetTypes.Reset();
	OutAssetTypes.Add(UUnLive2D::StaticClass());
	OutAssetTypes.Add(UUnLive2DAnimBase::StaticClass());
	OutAssetTypes.Add(UUnLive2DAnimBlueprint::StaticClass());
}

FAssetData FUnLive2DManagerAssetFamily::FindAssetOfType(UClass* InAssetClass) const
{
	if (InAssetClass)
	{
		if (InAssetClass->IsChildOf<UUnLive2D>())
		{
			return FAssetData(UnLive2D.Get());
		}
		else if (InAssetClass->IsChildOf<UUnLive2DAnimBase>())
		{
			if (UnLive2DAnimBase.IsValid())
			{
				return FAssetData(UnLive2DAnimBase.Get());
			}
			else
			{
				TArray<FAssetData> Assets;
				FindAssets<UUnLive2DAnimBase>(UnLive2D.Get(), Assets, "UnLive2D");
				if (Assets.IsValidIndex(0))
				{
					return Assets[0];
				}
			}
		}
		else if (InAssetClass->IsChildOf<UUnLive2DAnimBlueprint>())
		{
			if (UnLive2DAnimBlueprint.IsValid())
			{
				return FAssetData(UnLive2DAnimBlueprint.Get());
			}
			else
			{
				TArray<FAssetData> Assets;
				FindAssets<UUnLive2DAnimBlueprint>(UnLive2D.Get(), Assets, "TargetUnLive2D");
				if (Assets.IsValidIndex(0))
				{
					return Assets[0];
				}
			}
		}
	}

	return FAssetData();
}

void FUnLive2DManagerAssetFamily::FindAssetsOfType(UClass* InAssetClass, TArray<FAssetData>& OutAssets) const
{
	if (InAssetClass)
	{
		if (InAssetClass->IsChildOf<UUnLive2D>())
		{
			// we should always have a skeleton here, this asset family is based on it
			OutAssets.Add(FAssetData(UnLive2D.Get()));
		}
		else if (InAssetClass->IsChildOf<UUnLive2DAnimBase>())
		{
			FindAssets<UAnimationAsset>(UnLive2D.Get(), OutAssets, "UnLive2D");
		}
		else if (InAssetClass->IsChildOf<UUnLive2DAnimBlueprint>())
		{
			FindAssets<UUnLive2DAnimBlueprint>(UnLive2D.Get(), OutAssets, "TargetUnLive2D");
		}
	}
}

FText FUnLive2DManagerAssetFamily::GetAssetTypeDisplayName(UClass* InAssetClass) const
{
	if (InAssetClass)
	{
		if (InAssetClass->IsChildOf<UUnLive2D>())
		{
			return LOCTEXT("UnLive2DDisplayName", "Live2D");
		}
		else if (InAssetClass->IsChildOf<UUnLive2DAnimBase>())
		{
			return LOCTEXT("UnLive2DMotionDisplayName", "Live2D动作");
		}
		else if (InAssetClass->IsChildOf<UUnLive2DAnimBlueprint>())
		{
			return LOCTEXT("UnLive2DMotionDisplayName", "Live2D动画蓝图");
		}
	}

	return FText();
}

bool FUnLive2DManagerAssetFamily::IsAssetCompatible(const FAssetData& InAssetData) const
{
	UClass* Class = InAssetData.GetClass();
	if (Class)
	{
		if (Class->IsChildOf<UUnLive2D>())
		{
			return FAssetData(UnLive2D.Get()) == InAssetData;
		}
		else if (Class->IsChildOf<UUnLive2DAnimBase>())
		{
			FAssetDataTagMapSharedView::FFindTagResult Result = InAssetData.TagsAndValues.FindTag("UnLive2D");

			if (Result.IsSet())
			{
				return Result.GetValue() == FAssetData(UnLive2D.Get()).GetExportTextName();
			}
		}
		else if (Class->IsChildOf<UUnLive2DAnimBlueprint>())
		{
			FAssetDataTagMapSharedView::FFindTagResult Result = InAssetData.TagsAndValues.FindTag("TargetUnLive2D");

			if (Result.IsSet())
			{
				return Result.GetValue() == FAssetData(UnLive2D.Get()).GetExportTextName();
			}
		}
	}

	return false;
}

UClass* FUnLive2DManagerAssetFamily::GetAssetFamilyClass(UClass* InClass) const
{
	if (InClass)
	{
		if (InClass->IsChildOf<UUnLive2D>())
		{
			return UUnLive2D::StaticClass();
		}
		else if (InClass->IsChildOf<UUnLive2DAnimBase>())
		{
			return UUnLive2DAnimBase::StaticClass();
		}
		else if (InClass->IsChildOf<UUnLive2DAnimBlueprint>())
		{
			return UUnLive2DAnimBlueprint::StaticClass();
		}
	}

	return nullptr;
}

void FUnLive2DManagerAssetFamily::RecordAssetOpened(const FAssetData& InAssetData)
{
	if (IsAssetCompatible(InAssetData))
	{
		UClass* Class = InAssetData.GetClass();
		if (Class)
		{
			if (Class->IsChildOf<UUnLive2D>())
			{
				UnLive2D = Cast<UUnLive2D>(InAssetData.GetAsset());
			}
			else if (Class->IsChildOf<UUnLive2DAnimBase>())
			{
				UnLive2DAnimBase = Cast<UUnLive2DAnimBase>(InAssetData.GetAsset());
			}
			else if (Class->IsChildOf<UUnLive2DAnimBlueprint>())
			{
				UnLive2DAnimBlueprint = Cast<UUnLive2DAnimBlueprint>(InAssetData.GetAsset());
			}

			OnAssetOpened.Broadcast(InAssetData.GetAsset());
		}
	}
}

#undef LOCTEXT_NAMESPACE