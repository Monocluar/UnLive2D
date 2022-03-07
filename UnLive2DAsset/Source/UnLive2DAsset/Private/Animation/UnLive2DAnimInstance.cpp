#include "Animation/UnLive2DAnimInstance.h"

UUnLive2DAnimInstance::UUnLive2DAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUnLive2DAnimInstance::Serialize(FArchive& Ar)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_FUNC()

	Super::Serialize(Ar);

}

#if WITH_EDITOR
void UUnLive2DAnimInstance::PostEditUndo()
{
	Super::PostEditUndo();
}
#endif

