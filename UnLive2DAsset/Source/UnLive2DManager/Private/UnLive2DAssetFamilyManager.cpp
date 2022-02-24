#include "UnLive2DAssetFamilyManager.h"
#include "UnLive2DManagerAssetFamily.h"

FUnLive2DAssetFamilyManager& FUnLive2DAssetFamilyManager::Get()
{
	static FUnLive2DAssetFamilyManager TheManager;
	return TheManager;
}

TSharedRef<IUnLive2DAssetFamily> FUnLive2DAssetFamilyManager::CreatePersonaAssetFamily(const UObject* InAsset)
{
	// compact any invalid entries
	AssetFamilies.RemoveAll([](const TWeakPtr<class IUnLive2DAssetFamily>& InAssetFamily) { return !InAssetFamily.IsValid(); });

	// look for an existing matching asset family
	FAssetData AssetData(InAsset);
	for (TWeakPtr<class IUnLive2DAssetFamily>& AssetFamily : AssetFamilies)
	{
		if (AssetFamily.Pin()->IsAssetCompatible(AssetData))
		{
			return AssetFamily.Pin().ToSharedRef();
		}
	}

	// not found - make a new one
	TSharedRef<IUnLive2DAssetFamily> NewAssetFamily = MakeShareable(new FUnLive2DManagerAssetFamily(InAsset));
	AssetFamilies.Add(NewAssetFamily);
	return NewAssetFamily;
}

