
#pragma once

#include "CoreMinimal.h"
#include "AssetData.h"
#include "UObject/Class.h"


class IUnLive2DAssetFamily
{
public:

	virtual ~IUnLive2DAssetFamily() {}

	/** 获取此UnLive2D支持的所有资产类别（不需要包含派生类别） */
	virtual void GetAssetTypes(TArray<UClass*>& OutAssetTypes) const = 0;

	/** 查找指定类型中最相关的资产 */
	virtual FAssetData FindAssetOfType(UClass* AssetType) const = 0;

	/** 查找指定类型中最相关的资产 */
	template<typename AssetType>
	FAssetData GetAsset()
	{
		return FindAssetOfType(AssetType::StaticClass());
	}

	/** 查找指定类型中最相关的资产组 */
	virtual void FindAssetsOfType(UClass* AssetType, TArray<FAssetData>& OutAssets) const = 0;

	/** 查找指定类型中最相关的资产组 */
	template<typename AssetType>
	void GetAssets(TArray<FAssetData>& OutAssets)
	{
		FindAssetsOfType(AssetType::StaticClass(), OutAssets);
	}

	/** 获取将显示给用户的资源的名称 */
	virtual FText GetAssetTypeDisplayName(UClass* InAssetClass) const = 0;

	/** 检查资产是否与此组兼容 */
	virtual bool IsAssetCompatible(const FAssetData& InAssetData) const = 0;

	/** 返回此资产族传入类的最外层超类 */
	virtual UClass* GetAssetFamilyClass(UClass* InClass) const = 0;

	/** 记录资产已打开 */
	virtual void RecordAssetOpened(const FAssetData& InAssetData) = 0;

	/** 打开资产时激发的事件 */
	DECLARE_EVENT_OneParam(IUnLive2DAssetFamily, FOnUnLive2DAssetOpened, UObject*)
	virtual FOnUnLive2DAssetOpened& GetOnAssetOpened() = 0;

};
