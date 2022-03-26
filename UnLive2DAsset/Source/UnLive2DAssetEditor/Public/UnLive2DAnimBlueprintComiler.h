
#pragma once

#include "KismetCompilerModule.h"
#include "KismetCompiler.h"

class UNLIVE2DASSETEDITOR_API FUnLive2DAnimBlueprintComiler : public IBlueprintCompiler
{
public:
	/** IBlueprintCompiler interface */
	virtual bool CanCompile(const UBlueprint* Blueprint) override;
	virtual void Compile(UBlueprint* Blueprint, const FKismetCompilerOptions& CompileOptions, FCompilerResultsLog& Results) override;
};

class UNLIVE2DASSETEDITOR_API FUnLive2DAnimBlueprintComilerContext : public FKismetCompilerContext
{
public:
	FUnLive2DAnimBlueprintComilerContext(UBlueprint* SourceSketch, FCompilerResultsLog& InMessageLog, const FKismetCompilerOptions& InCompilerOptions)
		: FKismetCompilerContext(SourceSketch, InMessageLog, InCompilerOptions)
		, NewUnLive2DAnimBlueprintGeneratedClass(nullptr)
		, bIsDerivedAnimBlueprint(false)
	{

	}

protected:
	typedef FKismetCompilerContext Super;

	// FKismetCompilerContext interface
	virtual void ValidateLink(const UEdGraphPin* PinA, const UEdGraphPin* PinB) const override {}
	virtual void ValidatePin(const UEdGraphPin* Pin) const override {}
	virtual void ValidateNode(const UEdGraphNode* Node) const override {}
	virtual bool CanIgnoreNode(const UEdGraphNode* Node) const override { return true; }
	virtual bool ShouldForceKeepNode(const UEdGraphNode* Node) const override { return false; }
	virtual void PrecompileFunction(FKismetFunctionContext& Context, EInternalCompilerFlags InternalFlags) override {}
	virtual void PostCompile() override;
	virtual void CopyTermDefaultsToDefaultObject(UObject* DefaultObject) override;
	virtual void EnsureProperGeneratedClass(UClass*& TargetUClass) override;
	virtual void SpawnNewClass(const FString& NewClassName) override;
	virtual void OnNewClassSet(UBlueprintGeneratedClass* ClassToUse) override;
	virtual void PruneIsolatedNodes(const TArray<UEdGraphNode*>& RootSet, TArray<UEdGraphNode*>& GraphNodes) override {}
	virtual void CleanAndSanitizeClass(UBlueprintGeneratedClass* ClassToClean, UObject*& InOldCDO) override;
	virtual void MergeUbergraphPagesIn(UEdGraph* Ubergraph) override {};
	virtual void PreCompileUpdateBlueprintOnLoad(UBlueprint* BP) override;

private:
	/** the new class we are generating */
	class UUnLive2DAnimBlueprintGeneratedClass* NewUnLive2DAnimBlueprintGeneratedClass;

	// True if any parent class is also generated from an animation blueprint
	bool bIsDerivedAnimBlueprint;
};