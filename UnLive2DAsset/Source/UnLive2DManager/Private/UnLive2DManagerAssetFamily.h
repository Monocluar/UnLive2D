// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetData.h"
#include "IUnLive2DAssetFamily.h"

class UUnLive2D;
class UUnLive2DMotion;
class UUnLive2DAnimBlueprint;

class FUnLive2DManagerAssetFamily : public IUnLive2DAssetFamily
{
public:
	FUnLive2DManagerAssetFamily(const UObject* InFromObject);
	virtual ~FUnLive2DManagerAssetFamily() {}


public:
	static void FindCounterpartAssets(const UObject* InAsset, TWeakObjectPtr<const UUnLive2D>& OutUnLive2D);
	static void FindCounterpartAssets(const UObject* InAsset, const UUnLive2D*& OutUnLive2D);

protected:
	/** IUnLive2DAssetFamily interface */
	virtual void GetAssetTypes(TArray<UClass*>& OutAssetTypes) const override;
	virtual FAssetData FindAssetOfType(UClass* InAssetClass) const override;
	virtual void FindAssetsOfType(UClass* InAssetClass, TArray<FAssetData>& OutAssets) const override;
	virtual FText GetAssetTypeDisplayName(UClass* InAssetClass) const override;
	virtual bool IsAssetCompatible(const FAssetData& InAssetData) const override;
	virtual UClass* GetAssetFamilyClass(UClass* InClass) const override;
	virtual void RecordAssetOpened(const FAssetData& InAssetData) override;
	DECLARE_DERIVED_EVENT(FUnLive2DManagerAssetFamily, IUnLive2DAssetFamily::FOnUnLive2DAssetOpened, FOnUnLive2DAssetOpened)
	virtual FOnUnLive2DAssetOpened& GetOnAssetOpened() override { return OnAssetOpened; }

	virtual FSlateColor GetAssetTypeDisplayTint(UClass* InAssetClass) const override;

	virtual const FSlateBrush* GetAssetTypeDisplayIcon(UClass* InAssetClass) const override;

private:
	TWeakObjectPtr<const UUnLive2D> UnLive2D;

	TWeakObjectPtr<const UUnLive2DMotion> UnLive2DMotion;

	/** The last anim blueprint that was encountered */
	TWeakObjectPtr<const UUnLive2DAnimBlueprint> UnLive2DAnimBlueprint;

	/** Event fired when an asset is opened */
	FOnUnLive2DAssetOpened OnAssetOpened;
};