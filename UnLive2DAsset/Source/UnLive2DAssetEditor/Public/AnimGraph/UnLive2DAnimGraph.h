
#pragma once

#include "EdGraph/EdGraph.h"
#include "RigVMModel/RigVMGraph.h"
#include "UnLive2DAnimGraph.generated.h"

class URigVMGraph;

UCLASS()
class UNLIVE2DASSETEDITOR_API UUnLive2DAnimGraph : public UEdGraph
{
	GENERATED_BODY()

public:
	UUnLive2DAnimGraph();

private:
#if WITH_EDITORONLY_DATA

	UPROPERTY(transient)
		URigVMGraph* TemplateModel;

	UPROPERTY(transient)
		URigVMController* TemplateController;
#endif

	bool bSuspendModelNotifications;
	bool bIsSelecting;

private:

#if WITH_EDITOR

	UEdGraphNode* FindNodeForModelNodeName(const FName& InModelNodeName);
#endif

private:

	void HandleModifiedEvent(ERigVMGraphNotifType InNotifType, URigVMGraph* InGraph, UObject* InSubject);

#if WITH_EDITOR

	URigVMController* GetTemplateController();

	friend class UUnLive2DAnimUnitNodeSpawner;

#endif
};