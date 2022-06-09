#include "UnLive2DMotionFactory.h"
#include "Animation/UnLive2DMotion.h"
#include "UnLive2DMotionImportUI.h"
#include "Misc/FileHelper.h"
#include "UnLive2DMotionEditor/SUnLive2DMotionCrateDialog.h"

UUnLive2DMotionFactory::UUnLive2DMotionFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UUnLive2DMotion::StaticClass();

	Formats.Add(TEXT("json;Live2D Motion Format(motion3.json)"));

	bEditAfterNew = true;
	bEditorImport = true;
	bText = false;

	ImportPriority = DefaultImportPriority + 1;
}

bool UUnLive2DMotionFactory::DoesSupportClass(UClass* Class)
{
	return UUnLive2DMotion::StaticClass() == Class;
}

bool UUnLive2DMotionFactory::FactoryCanImport(const FString& Filename)
{
	const FString Extension = FPaths::GetExtension(Filename);

	if (Extension == TEXT("json"))
	{
		return Filename.Contains(TEXT(".motion3.json"));
	}

	return false;
}

UObject* UUnLive2DMotionFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	const FString Extension = FPaths::GetExtension(Filename);

	FString Live2DDataPath = Filename;
	if (!Filename.Contains(TEXT(".motion3.json")))
	{
		return nullptr;
	}
	ImportUISetting->AssetPath = *Live2DDataPath;
	TSharedRef<SUnLive2DMotionCrateDialog> Dialog = SNew(SUnLive2DMotionCrateDialog).ImportUI(ImportUISetting);
	if (!Dialog->ConfigureProperties(this))
	{
		return nullptr;
	}

	UUnLive2DMotion* Live2DMotion = NewObject<UUnLive2DMotion>(InParent, InClass, InName, Flags | RF_Transactional);

	if (! Live2DMotion->LoadLive2DMotionData(Live2DDataPath, ImportUISetting->MotionGroupType, ImportUISetting->MotionCount ))
	{
		Live2DMotion->RemoveFromRoot();
		return nullptr;
	}
	
	Live2DMotion->UnLive2D = TargetUnLive2D;
	return Live2DMotion;
}

void UUnLive2DMotionFactory::PostInitProperties()
{
	Super::PostInitProperties();
	ImportUISetting = NewObject<UUnLive2DMotionImportUI>(this, NAME_None, RF_NoFlags);
}

