// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SUnLive2DAssetFamilyShortcutBar : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUnLive2DAssetFamilyShortcutBar)
	{}
		
	SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs,  const TSharedRef<class FWorkflowCentricApplication>& InHostingApp, const TSharedRef<class IUnLive2DAssetFamily>& InAssetFamily);

private:
	/** 用于显示资产快捷方式的缩略图池 */
	TSharedPtr<class FAssetThumbnailPool> ThumbnailPool;

};