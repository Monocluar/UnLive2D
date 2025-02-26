#include "UnLive2DExpressionFactory.h"
#include "Animation/UnLive2DExpression.h"
#include "UnLive2DMotionEditor/SUnLive2DMotionCrateDialog.h"

UUnLive2DExpressionFactory::UUnLive2DExpressionFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UUnLive2DExpression::StaticClass();

	Formats.Add(TEXT("json;Live2D Expression Format(exp3.json)"));

	bEditAfterNew = true;
	bEditorImport = true;
	bText = false;

	ImportPriority = DefaultImportPriority + 1;
}

bool UUnLive2DExpressionFactory::DoesSupportClass(UClass* Class)
{
	return UUnLive2DExpression::StaticClass() == Class;
}

bool UUnLive2DExpressionFactory::FactoryCanImport(const FString& Filename)
{
	const FString Extension = FPaths::GetExtension(Filename);

	if (Extension == TEXT("json"))
	{
		return Filename.Contains(TEXT(".exp3.json"));
	}

	return false;
}

UObject* UUnLive2DExpressionFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	const FString Extension = FPaths::GetExtension(Filename);

	FString Live2DDataPath = Filename;
	if (!Filename.Contains(TEXT(".exp3.json")))
	{
		return nullptr;
	}
	TSharedRef<SUnLive2DMotionCrateDialog> Dialog = SNew(SUnLive2DMotionCrateDialog);
	if (!Dialog->ConfigureProperties(this))
	{
		return nullptr;
	}

	UUnLive2DExpression* Live2DExpression = NewObject<UUnLive2DExpression>(InParent, InClass, InName, Flags | RF_Transactional);

	if (!Live2DExpression->LoadLive2DExpressionData(Live2DDataPath))
	{
		Live2DExpression->RemoveFromRoot();
		return nullptr;
	}

	Live2DExpression->UnLive2D = TargetUnLive2D;
	return Live2DExpression;
}

