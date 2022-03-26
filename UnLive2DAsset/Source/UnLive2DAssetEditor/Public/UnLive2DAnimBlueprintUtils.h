
#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "EdGraph/EdGraphPin.h"

 class UBlueprint;

struct UNLIVE2DASSETEDITOR_API FUnLive2DAnimBlueprintUtils
{
	static FName ValidateName(UBlueprint* InBlueprint, const FString& InName);
};