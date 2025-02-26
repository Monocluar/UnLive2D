#pragma once

#include "CoreMinimal.h"
#include "Toolkits/IToolkitHost.h"
#include "AssetTypeActions/AssetTypeActions_Blueprint.h"

class UUnLive2DAnimBlueprint;

class FUnLive2DAnimBlurprintTypeAction : public FAssetTypeActions_Base
{
public:
	FUnLive2DAnimBlurprintTypeAction(uint32 InAssetCategory);

	// IAssetTypeActions Implementation
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual void GetActions(const TArray<UObject*>& InObjects, struct FToolMenuSection& Section) override;
	virtual void OpenAssetEditor( const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>() ) override;
	virtual uint32 GetCategories() override;
	virtual TSharedPtr<SWidget> GetThumbnailOverlay(const FAssetData& AssetData) const override;

private:

	void ExecuteFindUnLive2D(TArray<TWeakObjectPtr<UUnLive2DAnimBlueprint>> Objects);

	/** 上下文菜单项处理程序，用于更改提供的资产 */
	void RetargetAssets(TArray<UObject*> InAnimBlueprints, bool bDuplicateAssets);

private:
	uint32 AssetType;
};