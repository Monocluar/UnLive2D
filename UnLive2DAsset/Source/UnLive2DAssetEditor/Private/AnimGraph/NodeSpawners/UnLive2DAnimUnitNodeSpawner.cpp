#include "UnLive2DAnimUnitNodeSpawner.h"
#include "AnimGraph/UnLive2DAnimGraphNode_Base.h"
#include "RigVMCore/RigVMStruct.h"
#include "Animation/UnLive2DAnimBlueprint.h"
#include "AnimGraph/UnLive2DAnimGraph.h"
#include "BlueprintNodeTemplateCache.h"
#include "UnLive2DAnimBlueprintUtils.h"

UUnLive2DAnimUnitNodeSpawner* UUnLive2DAnimUnitNodeSpawner::CreateFromStruct(UScriptStruct* InStruct, const FText& InMenuDesc, const FText& InCategory, const FText& InTooltip)
{
	UUnLive2DAnimUnitNodeSpawner* NodeSpawner = NewObject<UUnLive2DAnimUnitNodeSpawner>(GetTransientPackage());
	NodeSpawner->StructTemplate = InStruct;
	NodeSpawner->NodeClass = UUnLive2DAnimGraphNode_Base::StaticClass();

	FBlueprintActionUiSpec& MenuSignature = NodeSpawner->DefaultMenuSignature;

	MenuSignature.MenuName = InMenuDesc;
	MenuSignature.Tooltip = InTooltip;
	MenuSignature.Category = InCategory;

	FString KeywordsMetadata, PrototypeNameMetadata;
	InStruct->GetStringMetaDataHierarchical(FRigVMStruct::KeywordsMetaName, &KeywordsMetadata);
	if (!PrototypeNameMetadata.IsEmpty())
	{
		if (KeywordsMetadata.IsEmpty())
		{
			KeywordsMetadata = PrototypeNameMetadata;
		}
		else
		{
			KeywordsMetadata = KeywordsMetadata + TEXT(",") + PrototypeNameMetadata;
		}
	}
	MenuSignature.Keywords = FText::FromString(KeywordsMetadata);

	if (MenuSignature.Keywords.IsEmpty())
	{
		// want to set it to something so we won't end up back in this condition
		MenuSignature.Keywords = FText::FromString(TEXT(" "));
	}

	// @TODO: should use details customization-like extensibility system to provide editor only data like this
	MenuSignature.Icon = FSlateIcon(TEXT("UnLive2DEditorStyle"), TEXT("UnLive2DAnim.RigUnit"));

	return NodeSpawner;
}

FBlueprintActionUiSpec UUnLive2DAnimUnitNodeSpawner::GetUiSpec(FBlueprintActionContext const& Context, FBindingSet const& Bindings) const
{
	UEdGraph* TargetGraph = (Context.Graphs.Num() > 0) ? Context.Graphs[0] : nullptr;
	FBlueprintActionUiSpec MenuSignature = PrimeDefaultUiSpec(TargetGraph);

	DynamicUiSignatureGetter.ExecuteIfBound(Context, Bindings, &MenuSignature);
	return MenuSignature;
}

UEdGraphNode* UUnLive2DAnimUnitNodeSpawner::Invoke(UEdGraph* ParentGraph, FBindingSet const& Bindings, FVector2D const Location) const
{
	UUnLive2DAnimGraphNode_Base* NewNode = nullptr;

	if (StructTemplate)
	{
#if WITH_EDITOR
		if (GEditor)
		{
			GEditor->CancelTransaction(0);
		}
#endif
		UBlueprint* Blueprint = CastChecked<UBlueprint>(ParentGraph->GetOuter());
		NewNode = SpawnNode(ParentGraph, Blueprint, StructTemplate, Location);
	}
	
	return NewNode;
}

bool UUnLive2DAnimUnitNodeSpawner::IsTemplateNodeFilteredOut(FBlueprintActionFilter const& Filter) const
{
	if (StructTemplate)
	{
		FString DeprecatedMetadata;
		StructTemplate->GetStringMetaDataHierarchical(FRigVMStruct::DeprecatedMetaName, &DeprecatedMetadata);
		if (!DeprecatedMetadata.IsEmpty())
		{
			return true;
		}
	}
	return Super::IsTemplateNodeFilteredOut(Filter);
}

UUnLive2DAnimGraphNode_Base* UUnLive2DAnimUnitNodeSpawner::SpawnNode(UEdGraph* ParentGraph, UBlueprint* Blueprint, UScriptStruct* StructTemplate, FVector2D const Location)
{
	UUnLive2DAnimGraphNode_Base* NewNode = nullptr;
	UUnLive2DAnimBlueprint* AnimBlueprint = Cast<UUnLive2DAnimBlueprint>(Blueprint);
	UUnLive2DAnimGraph* AnimGraph = Cast<UUnLive2DAnimGraph>(ParentGraph);

	if (AnimBlueprint != nullptr && AnimGraph != nullptr)
	{
		bool const bIsTemplateNode = FBlueprintNodeTemplateCache::IsTemplateOuter(ParentGraph);
		bool const bUndo = !bIsTemplateNode;

		FName Name = bIsTemplateNode ? *StructTemplate->GetDisplayNameText().ToString() : FUnLive2DAnimBlueprintUtils::ValidateName(AnimBlueprint, StructTemplate->GetFName().ToString());
		URigVMController* Controller = bIsTemplateNode ? AnimGraph->GetTemplateController() : AnimBlueprint->Controller;
	}

	return NewNode;
}

