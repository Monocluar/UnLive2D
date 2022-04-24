#include "Animation/UnLive2DAnimBlueprint.h"

#if WITH_EDITOR
#include "EdGraph/EdGraph.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintGraph.h"
#endif
#include "EngineUtils.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintNode_Base.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintNode_AssetReferencer.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"

#if WITH_EDITOR
TSharedPtr<class IUnLive2DAnimBlueprintAnimEditor> UUnLive2DAnimBlueprint::UnLive2DAnimBlueprintAnimEditor = nullptr;
#endif // WITH_EDITOR


UUnLive2DAnimBlueprint::UUnLive2DAnimBlueprint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUnLive2DAnimBlueprint::CacheAggregateValues()
{

}

void UUnLive2DAnimBlueprint::EvaluateNodes(bool bAddToRoot)
{

}

#if WITH_EDITOR

UEdGraph* UUnLive2DAnimBlueprint::GetGraph()
{
	return UnLive2DAnimBlueprintGraph;
}

TArray<UUnLive2DAnimBlueprintNode_Base*>& UUnLive2DAnimBlueprint::GetGraphAllNodes()
{
	return AllNodes;
}

void UUnLive2DAnimBlueprint::SeUnLive2DAnimBlueprintAnimEditor(TSharedPtr<IUnLive2DAnimBlueprintAnimEditor> InUnLive2DAnimBlueprintGraphEditor)
{
	check(!UnLive2DAnimBlueprintAnimEditor.IsValid());

	UnLive2DAnimBlueprintAnimEditor = InUnLive2DAnimBlueprintGraphEditor;
}

TSharedPtr<IUnLive2DAnimBlueprintAnimEditor> UUnLive2DAnimBlueprint::GetUnLive2DAnimBlueprintAnimEditor()
{
	return UnLive2DAnimBlueprintAnimEditor;
}

void UUnLive2DAnimBlueprint::PostInitProperties()
{
	Super::PostInitProperties();
	if (!HasAnyFlags(RF_ClassDefaultObject | RF_NeedLoad))
	{
		CreateGraph();
	}

}

void UUnLive2DAnimBlueprint::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	UUnLive2DAnimBlueprint* This = CastChecked<UUnLive2DAnimBlueprint>(InThis);

	Collector.AddReferencedObject(This->UnLive2DAnimBlueprintGraph, This);

	Super::AddReferencedObjects(InThis, Collector);
}

void UUnLive2DAnimBlueprint::CreateGraph()
{
	if (UnLive2DAnimBlueprintGraph != nullptr) return;

	UnLive2DAnimBlueprintGraph = UnLive2DAnimBlueprintAnimEditor->CreateUnLive2DAnimBlueprintGraph(this);
	UnLive2DAnimBlueprintGraph->bAllowDeletion = false;

	const UEdGraphSchema* Schema = UnLive2DAnimBlueprintGraph->GetSchema();
	Schema->CreateDefaultNodesForGraph(*UnLive2DAnimBlueprintGraph);
}

void UUnLive2DAnimBlueprint::PostLoad()
{
	Super::PostLoad();

#if WITH_EDITOR
	if (GIsEditor && !GetOutermost()->HasAnyPackageFlags(PKG_FilterEditorOnly))
	{
		if (ensure(UnLive2DAnimBlueprintGraph))
		{
			UUnLive2DAnimBlueprint::GetUnLive2DAnimBlueprintAnimEditor()->RemoveNullNodes(this);
		}

		for (UUnLive2DAnimBlueprintNode_Base* AnimBlueprintNode : AllNodes)
		{
			if (UUnLive2DAnimBlueprintNode_AssetReferencer* AssetReferencer = Cast<UUnLive2DAnimBlueprintNode_AssetReferencer>(AnimBlueprintNode))
			{
				AssetReferencer->LoadAsset();
			}
		}
	}
#endif
	if (GEngine && *GEngine->GameUserSettingsClass)
	{
		EvaluateNodes(false);
	}
	else
	{
		//OnPostEngineInitHandle = FCoreDelegates::OnPostEngineInit.AddUObject(this, &USoundCue::OnPostEngineInit);
	}

	CacheAggregateValues();

}

void UUnLive2DAnimBlueprint::Serialize(FStructuredArchive::FRecord Record)
{
	FArchive& UnderlyingArchive = Record.GetUnderlyingArchive();

	// 当我们保存或Cook时，更新数据
	if (UnderlyingArchive.IsSaving() || UnderlyingArchive.IsCooking())
	{
		CacheAggregateValues();
	}
#if WITH_EDITOR
	// 如果我们正在Cook，在序列化之前记录我们的Cook质量，然后撤销它。
	if (UnderlyingArchive.IsCooking() && UnderlyingArchive.IsSaving() && UnderlyingArchive.CookingTarget())
	{
		Super::Serialize(Record);
	}
	else
#endif
	{
		Super::Serialize(Record);
	}
#if WITH_EDITOR
	Record << SA_VALUE(TEXT("UnLive2DAnimBlueprintGraph"), UnLive2DAnimBlueprintGraph);
#endif
}

#endif
