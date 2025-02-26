
#pragma once

#include "AssetTypeActions_Base.h"


class FUnLive2DExpressionAction : public FAssetTypeActions_Base
{
public:
	FUnLive2DExpressionAction(uint32 InAssetCategory);

	// FAssetTypeActions_Base overrides
	virtual uint32 GetCategories() override;
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;

	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
private:
	uint32 AssetType;
};