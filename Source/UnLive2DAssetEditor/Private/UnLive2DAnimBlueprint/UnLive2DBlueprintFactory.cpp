#include "UnLive2DBlueprintFactory.h"
#include "UObject/Interface.h"
#include "Misc/MessageDialog.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "AssetRegistry/AssetData.h"
#include "Editor.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Animation/UnLive2DAnimBlueprint.h"
#include "ClassViewerFilter.h"
#include "UnLive2D.h"
#include "ClassViewerModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "SUnLive2DAssetCreateDialog.hpp"

#define LOCTEXT_NAMESPACE "FUnLive2DAssetEditorModule"

UUnLive2DBlueprintFactory::UUnLive2DBlueprintFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UUnLive2DAnimBlueprint::StaticClass();
}

bool UUnLive2DBlueprintFactory::ConfigureProperties()
{
	TSharedRef<SUnLive2DAssetCreateDialog<UUnLive2DBlueprintFactory>> Dialog = SNew(SUnLive2DAssetCreateDialog<UUnLive2DBlueprintFactory>);
	return Dialog->ConfigureProperties(this,LOCTEXT("CreateAnimBlueprintOptions", "创建UnLive2D动画蓝图"));
}

UObject* UUnLive2DBlueprintFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	// Make sure we are trying to factory a UnLive2D Anim Blueprint, then create and init one
	check(InClass->IsChildOf(UUnLive2DAnimBlueprint::StaticClass()));

	// If they selected an interface, we dont need a target UnLive2D
	if (TargetUnLive2D == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NeedValidSkeleton", "必须指定有效的UnLive2D数据."));
		return nullptr;
	}

	UUnLive2DAnimBlueprint* NewBP = NewObject<UUnLive2DAnimBlueprint>(InParent, InClass, InName, Flags | RF_Transactional);

	NewBP->TargetUnLive2D = TargetUnLive2D;

	return NewBP;
}

UObject* UUnLive2DBlueprintFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FactoryCreateNew(Class, InParent, Name, Flags, Context, Warn, NAME_None);
}

#undef LOCTEXT_NAMESPACE