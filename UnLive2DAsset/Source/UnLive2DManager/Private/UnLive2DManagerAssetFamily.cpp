#include "UnLive2DManagerAssetFamily.h"
#include "UnLive2D.h"

#define LOCTEXT_NAMESPACE "UnLive2DManagerAssetFamily"

FUnLive2DManagerAssetFamily::FUnLive2DManagerAssetFamily(const UObject* InFromObject)
	: UnLive2D(nullptr)
{
	if (InFromObject)
	{
		if (InFromObject->IsA<UUnLive2D>())
		{
			UnLive2D = CastChecked<UUnLive2D>(InFromObject);
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
}

void FUnLive2DManagerAssetFamily::GetAssetTypes(TArray<UClass*>& OutAssetTypes) const
{
	OutAssetTypes.Reset();
	OutAssetTypes.Add(UUnLive2D::StaticClass());
}

FAssetData FUnLive2DManagerAssetFamily::FindAssetOfType(UClass* InAssetClass) const
{
	if (InAssetClass)
	{
		if (InAssetClass->IsChildOf<UUnLive2D>())
		{
			return FAssetData(UnLive2D.Get());
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
	}
}

FText FUnLive2DManagerAssetFamily::GetAssetTypeDisplayName(UClass* InAssetClass) const
{
	if (InAssetClass)
	{
		if (InAssetClass->IsChildOf<UUnLive2D>())
		{
			return LOCTEXT("UnLive2DDisplayName", "UnLive2D");
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

			OnAssetOpened.Broadcast(InAssetData.GetAsset());
		}
	}
}

#undef LOCTEXT_NAMESPACE