
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "K2Node_BaseAsyncTask.h"
#include "K2Node_UnLive2DPlayMotion.generated.h"


UCLASS()
class UK2Node_UnLive2DRendererComponentPlayMotion : public UK2Node_BaseAsyncTask
{
	GENERATED_UCLASS_BODY()

protected:
	//~ Begin UEdGraphNode Interface
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	//~ Begin UK2Node Interface
	virtual FText GetMenuCategory() const override;
	//~ End UK2Node Interface
};

UCLASS()
class UK2Node_UnLive2DViewRendererUIPlayMotion : public UK2Node_BaseAsyncTask
{
	GENERATED_UCLASS_BODY()

protected:
	//~ Begin UEdGraphNode Interface
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	//~ Begin UK2Node Interface
	virtual FText GetMenuCategory() const override;
	//~ End UK2Node Interface
};