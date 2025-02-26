#include "AnimBlueprintGraph/UnLive2DAnimBlueprintNode_AssetReferencer.h"


bool UUnLive2DAnimBlueprintNode_AssetReferencer::ShouldHardReferenceAsset(const class ITargetPlatform* TargetPlatform) const
{
	return true;
}

#if WITH_EDITOR
void UUnLive2DAnimBlueprintNode_AssetReferencer::PostEditImport()
{
	Super::PostEditImport();

	LoadAsset();
}

#endif

