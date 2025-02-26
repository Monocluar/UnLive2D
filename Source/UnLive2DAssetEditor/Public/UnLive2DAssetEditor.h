// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "BlueprintActionDatabaseRegistrar.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUnLive2DEditor, Log, All);

class UNLIVE2DASSETEDITOR_API FUnLive2DAssetEditorModule : public IModuleInterface
{
public:
	static FORCEINLINE FUnLive2DAssetEditorModule& Get()
	{
		return FModuleManager::LoadModuleChecked< FUnLive2DAssetEditorModule >(TEXT("UnLive2DAssetEditor"));
	}


	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void OnPostEngineInit();

private:
	// 资源类型组
	uint32 GameAssetCategory;
};
