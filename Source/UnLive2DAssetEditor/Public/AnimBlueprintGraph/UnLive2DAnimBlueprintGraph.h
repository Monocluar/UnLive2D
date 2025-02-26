
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraph.h"
#include "UnLive2DAnimBlueprintGraph.generated.h"

class UUnLive2DAnimBlueprint;

UCLASS(MinimalAPI)
class UUnLive2DAnimBlueprintGraph : public UEdGraph
{
	GENERATED_UCLASS_BODY()

public:
	UUnLive2DAnimBlueprint* GetUnLive2DAnimBlueprint() const;

};