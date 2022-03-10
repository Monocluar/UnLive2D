#include "UnLive2DManagerAssetFamily.h"
#include "UnLive2D.h"
#include "UnLive2DMotion.h"
#include "Animation/UnLive2DAnimBlueprint.h"

#define LOCTEXT_NAMESPACE "UnLive2DManagerAssetFamily"

template<typename AssetType>
static void FindAssets(const UUnLive2D* InUnLive2D, TArray<FAssetData>& OutAssetData, FName UnLive2DTag)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FARFilter Filter;
	Filter.bRecursiveClasses = true;
	Filter.ClassNames.Add(AssetType::StaticClass()->GetFName());
	Filter.TagsAndValues.Add(UnLive2DTag, FAssetData(InUnLive2D).GetExportTextName());

	AssetRegistryModule.Get().GetAssets(Filter, OutAssetData);
}

FUnLive2DManagerAssetFamily::FUnLive2DManagerAssetFamily(const UObject* InFromObject)
	: UnLive2D(nullptr)
	, UnLive2DMotion(nullptr)
{
	if (InFromObject)
	{
		if (InFromObject->IsA<UUnLive2D>())
		{
			UnLive2D = CastChecked<UUnLive2D>(InFromObject);
		}
		else if (InFromObject->IsA<UUnLive2DMotion>())
		{
			UnLive2DMotion = CastChecked<UUnLive2DMotion>(InFromObject);
		}
		else if (InFromObject->IsA<UUnLive2DAnimBlueprint>())
		{
			UnLive2DAnimBlueprint = CastChecked<UUnLive2DAnimBlueprint>(InFromObject);
		}

		FindCounterpartAssets(InFromObject, UnLive2D);
	}
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
	else if (InAsset->IsA<UUnLive2DMotion>())
	{
		const UUnLive2DMotion* MotionAsset = CastChecked<const UUnLive2DMotion>(InAsset);
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
	OutAssetTypes.Add(UUnLive2DMotion::StaticClass());
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
		else if (InAssetClass->IsChildOf<UUnLive2DMotion>())
		{
			if (UnLive2DMotion.IsValid())
			{
				return FAssetData(UnLive2DMotion.Get());
			}
			else
			{
				TArray<FAssetData> Assets;
				FindAssets<UUnLive2DMotion>(UnLive2D.Get(), Assets, "UnLive2D");
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
		else if (InAssetClass->IsChildOf<UUnLive2DMotion>())
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
		else if (InAssetClass->IsChildOf<UUnLive2DMotion>())
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
		else if (Class->IsChildOf<UUnLive2DMotion>())
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
		else if (InClass->IsChildOf<UUnLive2DMotion>())
		{
			return UUnLive2DMotion::StaticClass();
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
			else if (Class->IsChildOf<UUnLive2DMotion>())
			{
				UnLive2DMotion = Cast<UUnLive2DMotion>(InAssetData.GetAsset());
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