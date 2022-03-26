#include "UnLive2DAnimBlueprintComiler.h"
#include "Stats/StatsHierarchical.h"
#include "Animation/UnLive2DAnimBlueprint.h"
#include "Kismet2/KismetReinstanceUtilities.h"
#include "KismetCompilerMisc.h"

bool FUnLive2DAnimBlueprintComiler::CanCompile(const UBlueprint* Blueprint)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_FUNC()

	if (Blueprint && Blueprint->ParentClass && Blueprint->ParentClass->IsChildOf(UUnLive2DAnimBlueprint::StaticClass()))
	{
		return true;
	}

	return false;
}

void FUnLive2DAnimBlueprintComiler::Compile(UBlueprint* Blueprint, const FKismetCompilerOptions& CompileOptions, FCompilerResultsLog& Results)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_FUNC()

	FUnLive2DAnimBlueprintComilerContext Compiler(Blueprint, Results, CompileOptions);
	Compiler.Compile();
}

void FUnLive2DAnimBlueprintComilerContext::PostCompile()
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_FUNC()

	UUnLive2DAnimBlueprint* UnLive2DAnimBlueprint = Cast<UUnLive2DAnimBlueprint>(Blueprint);
	if (UnLive2DAnimBlueprint)
	{
		if (!UnLive2DAnimBlueprint->bIsRegeneratingOnLoad)
		{
			// todo: can we find out somehow if we are cooking?
			UnLive2DAnimBlueprint->RecompileVM();
		}

	}

	{
		DECLARE_SCOPE_HIERARCHICAL_COUNTER(FKismetCompilerContext::PostCompile)
		FKismetCompilerContext::PostCompile();
	}
}

void FUnLive2DAnimBlueprintComilerContext::CopyTermDefaultsToDefaultObject(UObject* DefaultObject)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_FUNC()

	Super::CopyTermDefaultsToDefaultObject(DefaultObject);

	UUnLive2DAnimBlueprint* UnLive2DAnimBlueprint = Cast<UUnLive2DAnimBlueprint>(Blueprint);

	UUnLive2DAnimInstance* DefaultAnimInstance = Cast<UUnLive2DAnimInstance>(DefaultObject);

	if (bIsDerivedAnimBlueprint && DefaultAnimInstance)
	{
		// If we are a derived animation graph; apply any stored overrides.
		// Restore values from the root class to catch values where the override has been removed.
		UUnLive2DAnimBlueprintGeneratedClass* RootAnimClass = NewUnLive2DAnimBlueprintGeneratedClass;
		while (UUnLive2DAnimBlueprintGeneratedClass* NextClass = Cast<UUnLive2DAnimBlueprintGeneratedClass>(RootAnimClass->GetSuperClass()))
		{
			RootAnimClass = NextClass;
		}
		UObject* RootDefaultObject = RootAnimClass->GetDefaultObject();

		for (TFieldIterator<FProperty> It(RootAnimClass); It; ++It)
		{
			FProperty* RootProp = *It;

			if (FStructProperty* RootStructProp = CastField<FStructProperty>(RootProp))
			{
				/*if (RootStructProp->Struct->IsChildOf(FAnimNode_Base::StaticStruct()))
				{
					FStructProperty* ChildStructProp = FindFProperty<FStructProperty>(NewAnimBlueprintClass, *RootStructProp->GetName());
					check(ChildStructProp);
					uint8* SourcePtr = RootStructProp->ContainerPtrToValuePtr<uint8>(RootDefaultObject);
					uint8* DestPtr = ChildStructProp->ContainerPtrToValuePtr<uint8>(DefaultAnimInstance);
					check(SourcePtr && DestPtr);
					RootStructProp->CopyCompleteValue(DestPtr, SourcePtr);
				}*/
			}
		}
	}

	// Give game-specific logic a chance to replace animations
	if (DefaultAnimInstance)
	{
		DefaultAnimInstance->ApplyAnimOverridesToCDO(MessageLog);
	}

	if (bIsDerivedAnimBlueprint && DefaultAnimInstance)
	{

	}
}

void FUnLive2DAnimBlueprintComilerContext::EnsureProperGeneratedClass(UClass*& TargetUClass)
{
	if (TargetUClass && !((UObject*)TargetUClass)->IsA(UUnLive2DAnimBlueprintGeneratedClass::StaticClass()))
	{
		FKismetCompilerUtilities::ConsignToOblivion(TargetUClass, Blueprint->bIsRegeneratingOnLoad);
		TargetUClass = nullptr;
	}
}

void FUnLive2DAnimBlueprintComilerContext::SpawnNewClass(const FString& NewClassName)
{
	NewUnLive2DAnimBlueprintGeneratedClass = FindObject<UUnLive2DAnimBlueprintGeneratedClass>(Blueprint->GetOutermost(), *NewClassName);

	if (NewUnLive2DAnimBlueprintGeneratedClass == NULL)
	{
		NewUnLive2DAnimBlueprintGeneratedClass = NewObject<UUnLive2DAnimBlueprintGeneratedClass>(Blueprint->GetOutermost(), FName(*NewClassName), RF_Public | RF_Transactional);
	}
	else
	{
		// Already existed, but wasn't linked in the Blueprint yet due to load ordering issues
		FBlueprintCompileReinstancer::Create(NewUnLive2DAnimBlueprintGeneratedClass);
	}
	NewClass = NewUnLive2DAnimBlueprintGeneratedClass;

}

void FUnLive2DAnimBlueprintComilerContext::OnNewClassSet(UBlueprintGeneratedClass* ClassToUse)
{
	NewUnLive2DAnimBlueprintGeneratedClass = CastChecked<UUnLive2DAnimBlueprintGeneratedClass>(ClassToUse);
}

void FUnLive2DAnimBlueprintComilerContext::CleanAndSanitizeClass(UBlueprintGeneratedClass* ClassToClean, UObject*& InOldCDO)
{
	Super::CleanAndSanitizeClass(ClassToClean, InOldCDO);

	// Make sure our typed pointer is set
	check(ClassToClean == NewClass && NewUnLive2DAnimBlueprintGeneratedClass == NewClass);
}

void FUnLive2DAnimBlueprintComilerContext::PreCompileUpdateBlueprintOnLoad(UBlueprint* BP)
{
	if (UUnLive2DAnimBlueprint* RigBlueprint = Cast<UUnLive2DAnimBlueprint>(BP))
	{
		RigBlueprint->CreateMemberVariablesOnLoad();
	}
}

