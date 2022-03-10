// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "Modules/ModuleInterface.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Framework/Docking/TabManager.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"

class UUnLive2D;
class IUnLive2DToolkit;
class IUnLive2DAssetFamily;
class UUnLive2DMotion;
class UUnLive2DAnimBlueprint;

DECLARE_LOG_CATEGORY_EXTERN(LogUnLive2DManagerModule, Log, All);


/** Called back to register tabs */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnRegisterTabs, FWorkflowAllowedTabSet&, TSharedPtr<class FAssetEditorToolkit>);

/** Called back to register common layout extensions */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnRegisterLayoutExtensions, FLayoutExtender&);

// Called to invoke a specified tab
DECLARE_DELEGATE_OneParam(FOnInvokeTab, const FTabId&);


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
	virtual TSharedRef<IUnLive2DToolkit> CreatePersonaToolkit(UUnLive2DMotion* InMotionAsset) const;
	virtual TSharedRef<IUnLive2DToolkit> CreatePersonaToolkit(UUnLive2DAnimBlueprint* InAnimBlueprint) const;

	/** 为资产族创建快捷方式小部件 */
	virtual TSharedRef<SWidget> CreateAssetFamilyShortcutWidget(const TSharedRef<class FWorkflowCentricApplication>& InHostingApp, const TSharedRef<class IUnLive2DAssetFamily>& InAssetFamily) const;

public:
	
	/** 为提供的角色资产创建资产组 */
	virtual TSharedRef<IUnLive2DAssetFamily> CreatePersonaAssetFamily(const UObject* InAsset) const;

	/** Customize the details of a slot node for the specified details view */
	virtual void CustomizeBlueprintEditorDetails(const TSharedRef<class IDetailsView>& InDetailsView, FOnInvokeTab InOnInvokeTab);

	/** Register common layout extensions */
	virtual FOnRegisterLayoutExtensions& OnRegisterLayoutExtensions() { return OnRegisterLayoutExtensionsDelegate; }

	/** Register common tabs */
	virtual FOnRegisterTabs& OnRegisterTabs() { return OnRegisterTabsDelegate; }

private:
	
	TSharedPtr<FExtensibilityManager> MenuExtensibilityManager;
	TSharedPtr<FExtensibilityManager> ToolBarExtensibilityManager;

	/** Delegate broadcast to register common layout extensions */
	FOnRegisterLayoutExtensions OnRegisterLayoutExtensionsDelegate;

	/** Delegate broadcast to register common tabs */
	FOnRegisterTabs OnRegisterTabsDelegate;

};