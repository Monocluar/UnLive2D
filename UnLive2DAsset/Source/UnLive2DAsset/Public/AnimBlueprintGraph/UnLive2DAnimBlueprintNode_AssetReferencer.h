
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UnLive2DAnimBlueprintNode_Base.h"
#include "UnLive2DAnimBlueprintNode_AssetReferencer.generated.h"

UCLASS(Abstract)
class UUnLive2DAnimBlueprintNode_AssetReferencer : public UUnLive2DAnimBlueprintNode_Base
{
	GENERATED_BODY()

public:
	virtual void LoadAsset(bool bAddToRoot = false) PURE_VIRTUAL(UUnLive2DAnimBlueprintNode_AssetReferencer::LoadAsset,);

	virtual void ClearAssetReferences() PURE_VIRTUAL(UUnLive2DAnimBlueprintNode_AssetReferencer::ClearAssetReferences, );

#if WITH_EDITOR
	virtual void PostEditImport() override;
#endif
};