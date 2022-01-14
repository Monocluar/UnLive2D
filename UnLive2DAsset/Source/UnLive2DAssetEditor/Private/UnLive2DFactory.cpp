#include "UnLive2DFactory.h"
#include "UnLive2D.h"
#include "AssetImportTask.h"
#include "Misc/FeedbackContext.h"

#define LOCTEXT_NAMESPACE "FUnLive2DAssetEditorModule"

UUnLive2DFactory::UUnLive2DFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    SupportedClass = UUnLive2D::StaticClass();

	Formats.Add(TEXT("moc3;Live2D Format"));
	Formats.Add(TEXT("json;Live2D Format(Model3.json)"));

	//bCreateNew = true;
	bEditAfterNew = true;
	bEditorImport = true;
	bText         = false;

	ImportPriority = DefaultImportPriority + 1;
}

bool UUnLive2DFactory::DoesSupportClass(UClass* Class)
{
    return UUnLive2D::StaticClass() == Class;
}
/*

UObject* UUnLive2DFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UUnLive2D>(InParent, Class, Name, Flags | RF_Transactional);
}*/

bool UUnLive2DFactory::FactoryCanImport(const FString& Filename)
{
	const FString Extension = FPaths::GetExtension(Filename);

	if (Extension == TEXT("moc3"))
	{
		return true;
	}
	else if (Extension == TEXT("json"))
	{
		return Filename.Contains(TEXT(".model3.json"));
	}

	return false;
}

UObject* UUnLive2DFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	const FString Extension = FPaths::GetExtension(Filename);

	FString Live2DDataPath = Filename;
	if (Extension == TEXT("moc3"))
	{
		Live2DDataPath = Filename.LeftChop(Extension.Len()) + TEXT("model3.json");
	}
	else if (!Filename.Contains(TEXT(".model3.json")))
	{
		return nullptr;
	}

	UUnLive2D* UnLive2DPtr = NewObject<UUnLive2D>(InParent, InClass, InName, Flags | RF_Transactional);
	UnLive2DPtr->LoadLive2DFileDataFormPath(Live2DDataPath);

	return UnLive2DPtr;
}

#undef LOCTEXT_NAMESPACE