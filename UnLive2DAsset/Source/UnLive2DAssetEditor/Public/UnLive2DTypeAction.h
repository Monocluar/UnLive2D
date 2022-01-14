
#pragma once

#include "AssetTypeActions_Base.h"

class UNLIVE2DASSETEDITOR_API FUnLive2DTypeAction : public FAssetTypeActions_Base
{
public:
	FUnLive2DTypeAction(uint32 InAssetCategory);

	// FAssetTypeActions_Base overrides
	virtual uint32 GetCategories() override;
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;

	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
private:
	uint32 AssetType;
};