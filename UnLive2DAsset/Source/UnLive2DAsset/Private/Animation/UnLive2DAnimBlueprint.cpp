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
#include "Engine/AssetUserData.h"
#include "UnLive2DAssetModule.h"
#include "AnimBlueprintGraph/UnLive2DAnimBlueprintNode_Random.h"

#if WITH_EDITOR
TSharedPtr<class IUnLive2DAnimBlueprintAnimEditor> UUnLive2DAnimBlueprint::UnLive2DAnimBlueprintAnimEditor = nullptr;
#endif // WITH_EDITOR


UUnLive2DAnimBlueprint::UUnLive2DAnimBlueprint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Duration(-1.f)
{
}

void UUnLive2DAnimBlueprint::CacheAggregateValues()
{
	if (FirstNode)
	{
		FirstNode->ConditionalPostLoad();
		if (GIsEditor)
		{
			const float NewDuration = FirstNode->GetDuration();
#if WITH_EDITOR
			UE_LOG(LogUnLive2D, Display, TEXT("提示%s的缓存持续时间为零，已更改。考虑手动重新保存资产"),*GetFullName() );
#endif
			Duration = NewDuration;

		}
		bHasDelayNode = FirstNode->HasDelayNode();
		bHasConcatenatorNode = FirstNode->HasConcatenatorNode();
	}
}

void UUnLive2DAnimBlueprint::EvaluateNodes(bool bAddToRoot)
{

}

#if WITH_EDITOR

UEdGraph* UUnLive2DAnimBlueprint::GetGraph()
{
	return UnLive2DAnimBlueprintGraph;
}

void UUnLive2DAnimBlueprint::ResetGraph()
{
	for (UUnLive2DAnimBlueprintNode_Base* AnimBlueprintNode : AllNodes)
	{
		UnLive2DAnimBlueprintGraph->RemoveNode(AnimBlueprintNode->GraphNode);
	}
	AllNodes.Reset();
	FirstNode = nullptr;
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

void UUnLive2DAnimBlueprint::RecursivelySetExcludeBranchCulling(UUnLive2DAnimBlueprintNode_Base* CurrentNode)
{
	if (CurrentNode == nullptr) return;

	UUnLive2DAnimBlueprintNode_Random* RandomNode = Cast<UUnLive2DAnimBlueprintNode_Random>(CurrentNode);
	if (RandomNode)
	{
		RandomNode->bAnimBlueprintExcludedFromBranchCulling = bExcludeFromRandomNodeBranchCulling;
		RandomNode->MarkPackageDirty();
	}
	for (UUnLive2DAnimBlueprintNode_Base* ChildNode : CurrentNode->ChildNodes)
	{
		RecursivelySetExcludeBranchCulling(ChildNode);
	}
}

void UUnLive2DAnimBlueprint::PostInitProperties()
{
	Super::PostInitProperties();
	if (!HasAnyFlags(RF_ClassDefaultObject | RF_NeedLoad))
	{
		CreateGraph();
	}
	CacheAggregateValues();
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

void UUnLive2DAnimBlueprint::ClearGraph()
{
	if (UnLive2DAnimBlueprintGraph == nullptr) return;

	UnLive2DAnimBlueprintGraph->Nodes.Empty();

	const UEdGraphSchema* Schema = UnLive2DAnimBlueprintGraph->GetSchema();
	Schema->CreateDefaultNodesForGraph(*UnLive2DAnimBlueprintGraph);
}

void UUnLive2DAnimBlueprint::SetupAnimNode(UUnLive2DAnimBlueprintNode_Base* InAnimNode, bool bSelectNewNode /*= true*/)
{
	if (UnLive2DAnimBlueprintAnimEditor == nullptr) return;

	check(InAnimNode->GraphNode == NULL);

	UnLive2DAnimBlueprintAnimEditor->SetupAnimBlueprintNode(UnLive2DAnimBlueprintGraph, InAnimNode, bSelectNewNode);
}

void UUnLive2DAnimBlueprint::LinkGraphNodesFromUnLive2DAnimNodes()
{
	if (UnLive2DAnimBlueprintAnimEditor == nullptr) return;

	UnLive2DAnimBlueprintAnimEditor->LinkGraphNodesFromAnimBlueprintNodes(this);
	CacheAggregateValues();
}

void UUnLive2DAnimBlueprint::CompileUnLive2DAnimNodesFromGraphNodes()
{
	UnLive2DAnimBlueprintAnimEditor->CompileAnimBlueprintNodesFromGraphNodes(this);
}

#endif

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

void UUnLive2DAnimBlueprint::AddAssetUserData(UAssetUserData* InUserData)
{
	if (InUserData != nullptr)
	{
		UAssetUserData* ExistingData = GetAssetUserDataOfClass(InUserData->GetClass());
		if (ExistingData != nullptr)
		{
			AssetUserData.Remove(ExistingData);
		}
		AssetUserData.Add(InUserData);
	}
}

void UUnLive2DAnimBlueprint::RemoveUserDataOfClass(TSubclassOf<UAssetUserData> InUserDataClass)
{
	for (int32 DataIdx = 0; DataIdx < AssetUserData.Num(); DataIdx++)
	{
		UAssetUserData* Datum = AssetUserData[DataIdx];
		if (Datum != nullptr && Datum->IsA(InUserDataClass))
		{
			AssetUserData.RemoveAt(DataIdx);
			return;
		}
	}
}

UAssetUserData* UUnLive2DAnimBlueprint::GetAssetUserDataOfClass(TSubclassOf<UAssetUserData> InUserDataClass)
{
	for (int32 DataIdx = 0; DataIdx < AssetUserData.Num(); DataIdx++)
	{
		UAssetUserData* Datum = AssetUserData[DataIdx];
		if (Datum != nullptr && Datum->IsA(InUserDataClass))
		{
			return Datum;
		}
	}
	return nullptr;
}

const TArray<UAssetUserData*>* UUnLive2DAnimBlueprint::GetAssetUserDataArray() const
{
	return &AssetUserData;
}

