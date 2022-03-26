
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/UnrealType.h"
#include "Editor.h"
#include "K2Node.h"
#include "Animation/UnLive2DAnimNodeBase.h"
#include "UnLive2DAnimGraphNode_Base.generated.h"


UCLASS(Abstract)
class UNLIVE2DASSETEDITOR_API UUnLive2DAnimGraphNode_Base : public UK2Node
{
	GENERATED_UCLASS_BODY()

public:

	/** Called to propagate data from the internal node to the preview in Persona. */
	virtual void CopyNodeDataToPreviewNode(FUnLive2DAnimNode_Base* InPreviewNode) {}

	/**
	 * Selection notification callback.
	 * If a node needs to handle viewport input etc. then it should push an editor mode here.
	 * @param	bInIsSelected	Whether we selected or deselected the node
	 * @param	InModeTools		The mode tools. Use this to push the editor mode if required.
	 * @param	InRuntimeNode	The runtime node to go with this skeletal control. This may be NULL in some cases when bInIsSelected is false.
	 */
	virtual void OnNodeSelected(bool bInIsSelected, class FEditorModeTools& InModeTools, struct FUnLive2DAnimNode_Base* InRuntimeNode);

protected:
	// UObject interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	/*virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void Serialize(FArchive& Ar) override;*/
	// End of UObject interface

private:

	UPROPERTY()
		FString ModelNodePath;

	friend class UUnLive2DAnimGraph;
};