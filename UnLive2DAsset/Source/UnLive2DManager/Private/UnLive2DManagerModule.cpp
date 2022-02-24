#include "UnLive2DManagerModule.h"
#include "IUnLive2DToolkit.h"
#include "UnLive2DManagerToolkit.h"
#include "UnLive2DAssetFamilyManager.h"
#include "Modules/ModuleManager.h"
#include "SUnLive2DAssetFamilyShortcutBar.h"

DEFINE_LOG_CATEGORY(LogUnLive2DManagerModule);

void FUnLive2DManagerModule::StartupModule()
{
	MenuExtensibilityManager = MakeShareable(new FExtensibilityManager);
	ToolBarExtensibilityManager = MakeShareable(new FExtensibilityManager);
	// Make sure the advanced preview scene module is loaded 
	FModuleManager::Get().LoadModuleChecked("AdvancedPreviewScene");
}

void FUnLive2DManagerModule::ShutdownModule()
{
	MenuExtensibilityManager.Reset();
	ToolBarExtensibilityManager.Reset();
}

TSharedRef<IUnLive2DToolkit> FUnLive2DManagerModule::CreatePersonaToolkit(UUnLive2D* InAsset) const
{
	TSharedRef<FUnLive2DMangerToolkit> NewPersonaToolkit(new FUnLive2DMangerToolkit());
	NewPersonaToolkit->Initialize(InAsset);
	NewPersonaToolkit->CreatePreviewScene();
	return NewPersonaToolkit;
}

TSharedRef<SWidget> FUnLive2DManagerModule::CreateAssetFamilyShortcutWidget(const TSharedRef<class FWorkflowCentricApplication>& InHostingApp, const TSharedRef<class IUnLive2DAssetFamily>& InAssetFamily) const
{
	return SNew(SUnLive2DAssetFamilyShortcutBar, InHostingApp, InAssetFamily);
}

TSharedRef<IUnLive2DAssetFamily> FUnLive2DManagerModule::CreatePersonaAssetFamily(const UObject* InAsset) const
{
	return FUnLive2DAssetFamilyManager::Get().CreatePersonaAssetFamily(InAsset);
}


IMPLEMENT_MODULE(FUnLive2DManagerModule, UnLive2DManager)
