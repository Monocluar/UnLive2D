// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "Modules/ModuleInterface.h"
#include "Toolkits/AssetEditorToolkit.h"

class UUnLive2D;
class IUnLive2DToolkit;
class IUnLive2DAssetFamily;

DECLARE_LOG_CATEGORY_EXTERN(LogUnLive2DManagerModule, Log, All);


class FUnLive2DManagerModule : public IModuleInterface, public IHasMenuExtensibility
{
public:
	/**
	 * Called right after the module's DLL has been loaded and the module object has been created
	 */
	virtual void StartupModule();

	/**
	 * Called before the module is unloaded, right before the module object is destroyed.
	 */
	virtual void ShutdownModule();

	
	virtual TSharedPtr<FExtensibilityManager> GetMenuExtensibilityManager() {return MenuExtensibilityManager;}
	virtual TSharedPtr<FExtensibilityManager> GetToolBarExtensibilityManager() {return ToolBarExtensibilityManager;}

public:
	// 创建一个工具包
	virtual TSharedRef<IUnLive2DToolkit> CreatePersonaToolkit(UUnLive2D* InAsset) const;

	/** 为资产族创建快捷方式小部件 */
	virtual TSharedRef<SWidget> CreateAssetFamilyShortcutWidget(const TSharedRef<class FWorkflowCentricApplication>& InHostingApp, const TSharedRef<class IUnLive2DAssetFamily>& InAssetFamily) const;

public:
	
	/** 为提供的角色资产创建资产组 */
	virtual TSharedRef<IUnLive2DAssetFamily> CreatePersonaAssetFamily(const UObject* InAsset) const;

private:
	
	TSharedPtr<FExtensibilityManager> MenuExtensibilityManager;
	TSharedPtr<FExtensibilityManager> ToolBarExtensibilityManager;

};