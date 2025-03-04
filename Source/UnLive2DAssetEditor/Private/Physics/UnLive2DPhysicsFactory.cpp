#include "Physics/UnLive2DPhysicsFactory.h"
#include "Physics/UnLive2DPhysics.h"
#include "SUnLive2DNorCreateDialog.hpp"

UUnLive2DPhysicsFactory::UUnLive2DPhysicsFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UUnLive2DPhysics::StaticClass();

	Formats.Add(TEXT("json;Live2D Physics Format(physics3.json)"));

	bEditAfterNew = true;
	bEditorImport = true;
	bText = false;

	ImportPriority = DefaultImportPriority + 1;
}


bool UUnLive2DPhysicsFactory::DoesSupportClass(UClass* Class)
{
	return UUnLive2DPhysics::StaticClass() == Class;
}


bool UUnLive2DPhysicsFactory::FactoryCanImport(const FString& Filename)
{
	const FString Extension = FPaths::GetExtension(Filename);

	if (Filename.EndsWith(".physics3.json")) return true;

	return false;
}

UObject* UUnLive2DPhysicsFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	const FString Extension = FPaths::GetExtension(Filename);

	FString Live2DDataPath = Filename;
	if (!Filename.Contains(TEXT(".physics3.json")))
	{
		return nullptr;
	}
	FString FileInName = InName.ToString().Replace(TEXT("."), TEXT("_"));
	UUnLive2DPhysics* Live2DPhysics = NewObject<UUnLive2DPhysics>(InParent, InClass, *FileInName, Flags | RF_Transactional);
	if (!Live2DPhysics->LoadLive2DPhysicsData(Live2DDataPath))
	{
		Live2DPhysics->RemoveFromRoot();
		return nullptr;
	}
	return Live2DPhysics;
}
