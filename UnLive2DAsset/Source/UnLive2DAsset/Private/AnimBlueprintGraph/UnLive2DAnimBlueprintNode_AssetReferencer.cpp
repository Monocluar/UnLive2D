#include "AnimBlueprintGraph/UnLive2DAnimBlueprintNode_AssetReferencer.h"

#if WITH_EDITOR

void UUnLive2DAnimBlueprintNode_AssetReferencer::PostEditImport()
{
	Super::PostEditImport();

	LoadAsset();
}

#endif

