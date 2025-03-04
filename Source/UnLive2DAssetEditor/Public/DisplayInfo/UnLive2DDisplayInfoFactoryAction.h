#pragma once

#include "AssetTypeActions_Base.h"


class FUnLive2DDisplayInfoFactoryAction : public FAssetTypeActions_Base
{
public:
	FUnLive2DDisplayInfoFactoryAction(uint32 InAssetCategory);

protected:
	// FAssetTypeActions_Base overrides
	virtual uint32 GetCategories() override;
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;

private:
	uint32 AssetType;
};