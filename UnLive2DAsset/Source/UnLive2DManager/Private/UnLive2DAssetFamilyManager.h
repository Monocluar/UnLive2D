
#pragma once

#include "CoreMinimal.h"

class IUnLive2DAssetFamily;

class FUnLive2DAssetFamilyManager
{
public:

	static FUnLive2DAssetFamilyManager& Get();

	TSharedRef<IUnLive2DAssetFamily> CreatePersonaAssetFamily(const UObject* InAsset);

private:
	FUnLive2DAssetFamilyManager(){}

private:
	TArray<TWeakPtr<IUnLive2DAssetFamily>> AssetFamilies;
};