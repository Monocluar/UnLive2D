#include "Animation/UnLive2DAnimBlueprint.h"
#include "Animation/UnLive2DAnimBlueprintGeneratedClass.h"
#include "Animation/UnLive2DAnimInstance.h"

UUnLive2DAnimBlueprint::UUnLive2DAnimBlueprint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

class UUnLive2DAnimBlueprintGeneratedClass* UUnLive2DAnimBlueprint::GetAnimBlueprintGeneratedClass() const
{
	UUnLive2DAnimBlueprintGeneratedClass* Result = Cast<UUnLive2DAnimBlueprintGeneratedClass>(*GeneratedClass);
	return Result;
}

UUnLive2DAnimBlueprintGeneratedClass* UUnLive2DAnimBlueprint::GetUnLive2DAnimBlueprintClass() const
{
	UUnLive2DAnimBlueprintGeneratedClass* Result = Cast<UUnLive2DAnimBlueprintGeneratedClass>(*SkeletonGeneratedClass);
	return Result;
}

#if WITH_EDITOR

UClass* UUnLive2DAnimBlueprint::GetBlueprintClass() const
{
	return UUnLive2DAnimBlueprintGeneratedClass::StaticClass();
}

#endif

UUnLive2DAnimBlueprint* UUnLive2DAnimBlueprint::GetPreviewAnimationBlueprint() const
{
#if WITH_EDITORONLY_DATA
	/*if (!PreviewAnimationBlueprint.IsValid())
	{
		PreviewAnimationBlueprint.LoadSynchronous();
	}
	return PreviewAnimationBlueprint.Get();*/
	return nullptr;
#else
	return nullptr;
#endif
}
