#include "UnLive2DExpressionFactory.h"
#include "Animation/UnLive2DExpression.h"
#include "UnLive2DMotionEditor/SUnLive2DMotionCrateDialog.h"
#include "SUnLive2DAssetCreateDialog.hpp"

UUnLive2DExpressionFactory::UUnLive2DExpressionFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UUnLive2DExpression::StaticClass();

	Formats.Add(TEXT("json;Live2D Expression Format(exp3.json)"));

	bEditAfterNew = true;
	bEditorImport = true;
	bText = false;
	bCreateNew = true;

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

bool UUnLive2DExpressionFactory::ConfigureProperties()
{
	TSharedRef<SUnLive2DAssetCreateDialog<UUnLive2DExpressionFactory>> Dialog = SNew(SUnLive2DAssetCreateDialog<UUnLive2DExpressionFactory>);
	return Dialog->ConfigureProperties(this, NSLOCTEXT("FUnLive2DAssetEditorModule", "CreateExpressionOptions", "创建UnLive2D表情"));
}

UObject* UUnLive2DExpressionFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	// Make sure we are trying to factory a UnLive2D Anim Blueprint, then create and init one
	check(InClass->IsChildOf(UUnLive2DExpression::StaticClass()));

	if (TargetUnLive2D == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("FUnLive2DAssetEditorModule","NeedValidSkeleton", "必须指定有效的UnLive2D数据."));
		return nullptr;
	}

	UUnLive2DExpression* Expression = NewObject<UUnLive2DExpression>(InParent, InClass, InName, Flags | RF_Transactional);

	Expression->UnLive2D = TargetUnLive2D;
	static FUnLiveByteData UnLiveByteData;
	if (UnLiveByteData.ByteData.Num() == 0)
	{
		FString ExpressionJsonTemplate = TEXT("{\"Type\": \"Live2D Expression\",\"Parameters\": []}");
		UnLiveByteData.ByteData.SetNum(ExpressionJsonTemplate.Len());
		FMemory::Memmove(UnLiveByteData.ByteData.GetData(), TCHAR_TO_ANSI(*ExpressionJsonTemplate), ExpressionJsonTemplate.Len());
	}
	Expression->SetLive2DExpressionData(UnLiveByteData);

	return Expression;
}

UObject* UUnLive2DExpressionFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FactoryCreateNew(Class, InParent, Name, Flags, Context, Warn, NAME_None);
}

