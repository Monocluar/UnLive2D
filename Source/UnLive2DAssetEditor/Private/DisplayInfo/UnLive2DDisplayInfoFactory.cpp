#include "DisplayInfo/UnLive2DDisplayInfoFactory.h"
#include "Editor/EditorUnLive2DDisplayInfo.h"
#include "Misc/FileHelper.h"


UUnLive2DDisplayInfoFactory::UUnLive2DDisplayInfoFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UEditorUnLive2DDisplayInfo::StaticClass();

	Formats.Add(TEXT("json;Live2D Display Info Format(cdi3.json)"));

	bEditAfterNew = true;
	bEditorImport = true;
	bText = false;

	ImportPriority = DefaultImportPriority + 1;
}

bool UUnLive2DDisplayInfoFactory::DoesSupportClass(UClass* Class)
{
	return UEditorUnLive2DDisplayInfo::StaticClass() == Class;
}

bool UUnLive2DDisplayInfoFactory::FactoryCanImport(const FString& Filename)
{
	const FString Extension = FPaths::GetExtension(Filename);

	if (Filename.EndsWith(".cdi3.json")) return true;

	return false;
}

UObject* UUnLive2DDisplayInfoFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	const FString Extension = FPaths::GetExtension(Filename);

	FString Live2DDataPath = Filename;
	if (!Filename.Contains(TEXT(".cdi3.json")))
	{
		return nullptr;
	}
	FString FileContent;
	const bool ReadSuc = FFileHelper::LoadFileToString(FileContent, *Live2DDataPath);
	if (!ReadSuc) return nullptr;


	FString FileInName = InName.ToString().Replace(TEXT("."), TEXT("_"));
	UEditorUnLive2DDisplayInfo* Live2DPhysics = NewObject<UEditorUnLive2DDisplayInfo>(InParent, InClass, *FileInName, Flags | RF_Transactional);
	if (!Live2DPhysics->LoadLive2DDisplayInfo(FileContent))
	{
		Live2DPhysics->RemoveFromRoot();
		return nullptr;
	}
	return Live2DPhysics;
}
