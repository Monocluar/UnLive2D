#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/Blueprint.h"
#include "UnLive2DMotion.h"
#include "UnLive2DAnimBlueprint.generated.h"

class UUnLive2DAnimBlueprintGeneratedClass;


UCLASS(BlueprintType)
class UNLIVE2DASSET_API UUnLive2DAnimBlueprint : public UBlueprint
{
	GENERATED_UCLASS_BODY()

public:

	UUnLive2DAnimBlueprintGeneratedClass* GetAnimBlueprintGeneratedClass() const;

	UUnLive2DAnimBlueprintGeneratedClass* GetUnLive2DAnimBlueprintClass() const;

public:
	// 目标UnLive2D数据资源
	UPROPERTY(AssetRegistrySearchable, EditAnywhere, AdvancedDisplay, Category=ClassOptions)
		UUnLive2D* TargetUnLive2D;

protected:

#if WITH_EDITOR

	virtual UClass* GetBlueprintClass() const override;

	virtual bool SupportedByDefaultBlueprintFactory() const override { return false; }
	virtual bool IsValidForBytecodeOnlyRecompile() const override { return false; }
	// End of UBlueprint interface


#endif



};