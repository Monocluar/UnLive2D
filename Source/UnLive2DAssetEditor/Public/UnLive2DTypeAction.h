
#pragma once

#include "AssetTypeActions_Base.h"

class UUnLive2D;

class UNLIVE2DASSETEDITOR_API FUnLive2DTypeAction : public FAssetTypeActions_Base
{
public:
	FUnLive2DTypeAction(uint32 InAssetCategory);

	// FAssetTypeActions_Base overrides
	virtual uint32 GetCategories() override;
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return true; }
	virtual void GetActions(const TArray<UObject*>& InObjects, struct FToolMenuSection& Section) override;

	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;

private:

	void FillCreateMenu(FMenuBuilder& MenuBuilder, TArray<TWeakObjectPtr<UUnLive2D>> UnLive2DArr);

	void CreateNewAnimBlueprint(TArray<TWeakObjectPtr<UUnLive2D>> UnLive2DArr);
	void CreateNewExpression(TArray<TWeakObjectPtr<UUnLive2D>> UnLive2DArr);

private:
	template<class FactoryClass>
	void CreateNewAsset(TArray<TWeakObjectPtr<UUnLive2D>> UnLive2DArr,const FString& DefaultSuffix) const;

private:
	uint32 AssetType;
};
