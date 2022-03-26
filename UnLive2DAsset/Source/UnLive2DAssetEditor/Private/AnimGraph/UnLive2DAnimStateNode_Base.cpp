#include "AnimGraph/UnLive2DAnimStateNode_Base.h"
#include "UnLive2DAssetEditor.h"
#include "UObject/FrameworkObjectVersion.h"

UUnLive2DAnimStateNode_Base::UUnLive2DAnimStateNode_Base(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUnLive2DAnimStateNode_Base::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	Ar.UsingCustomVersion(FFrameworkObjectVersion::GUID);
}

void UUnLive2DAnimStateNode_Base::PostLoad()
{
	Super::PostLoad();

	const int32 CustomVersion = GetLinkerCustomVersion(FFrameworkObjectVersion::GUID);

	if (CustomVersion < FFrameworkObjectVersion::FixNonTransactionalPins)
	{
		int32 BrokenPinCount = 0;
		for (UEdGraphPin_Deprecated* Pin : DeprecatedPins)
		{
			if (!Pin->HasAnyFlags(RF_Transactional))
			{
				++BrokenPinCount;
				Pin->SetFlags(Pin->GetFlags() | RF_Transactional);
			}
		}

		if (BrokenPinCount > 0)
		{
			UE_LOG(LogUnLive2DEditor, Log, TEXT("Fixed %d non-transactional pins in %s"), BrokenPinCount, *GetName());
		}
	}
}

