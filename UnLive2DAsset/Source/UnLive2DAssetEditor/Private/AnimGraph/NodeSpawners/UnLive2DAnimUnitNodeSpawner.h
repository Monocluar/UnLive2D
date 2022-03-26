
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "BlueprintNodeSpawner.h"
#include "UnLive2DAnimUnitNodeSpawner.generated.h"

class UUnLive2DAnimGraphNode_Base;

UCLASS(Transient)
class UNLIVE2DASSETEDITOR_API UUnLive2DAnimUnitNodeSpawner : public UBlueprintNodeSpawner
{
	GENERATED_BODY()

public:
	static UUnLive2DAnimUnitNodeSpawner* CreateFromStruct(UScriptStruct* InStruct, const FText& InMenuDesc, const FText& InCategory, const FText& InTooltip);

public:
	// UBlueprintNodeSpawner interface
	virtual void Prime() override {};
	virtual FBlueprintActionUiSpec GetUiSpec(FBlueprintActionContext const& Context, FBindingSet const& Bindings) const override;
	virtual UEdGraphNode* Invoke(UEdGraph* ParentGraph, FBindingSet const& Bindings, FVector2D const Location) const override;
	virtual bool IsTemplateNodeFilteredOut(FBlueprintActionFilter const& Filter) const override;
	// End UBlueprintNodeSpawner interface

private:
	/** The unit type we will spawn */
	UPROPERTY(Transient)
		UScriptStruct* StructTemplate;


	static UUnLive2DAnimGraphNode_Base* SpawnNode(UEdGraph* ParentGraph, UBlueprint* Blueprint, UScriptStruct* StructTemplate, FVector2D const Location);
};