// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Stats/Stats.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUnLive2D, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogLive2D, Log, All);

class FUnLive2DAssetModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

DECLARE_STATS_GROUP(TEXT("UnLive2D"), STATGROUP_UnLive2D, STATCAT_Advanced);
DECLARE_CYCLE_STAT_EXTERN(TEXT("SetUnLive2D (RT)"), STAT_PaperRender_SetSpriteRT, STATGROUP_UnLive2D, UNLIVE2DASSET_API);