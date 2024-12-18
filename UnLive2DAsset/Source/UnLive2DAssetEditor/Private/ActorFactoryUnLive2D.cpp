#include "ActorFactoryUnLive2D.h"
#include "UnLive2D.h"
#include "UnLive2DActor.h"
#include "UnLive2DRendererComponent.h"

UActorFactoryUnLive2D::UActorFactoryUnLive2D(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("UnLive2D", "UnLive2DDisplayName", "UnLive2D");
	NewActorClass = AUnLive2DActor::StaticClass();
}

void UActorFactoryUnLive2D::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	Super::PostSpawnActor(Asset, NewActor);

	UUnLive2D* UnLive2D = CastChecked<UUnLive2D>(Asset);
	AUnLive2DActor* UnLive2DActor = CastChecked<AUnLive2DActor>(NewActor);

	UnLive2DActor->GetUnLive2DRendererComponent()->UnregisterComponent();

	UnLive2DActor->GetUnLive2DRendererComponent()->SetUnLive2D(UnLive2D);

	if (UnLive2DActor->GetWorld()->HasBegunPlay() && UnLive2DActor->GetWorld()->GetNetMode() != NM_Client)
	{
		UnLive2DActor->SetReplicates(true);
		UnLive2DActor->bAlwaysRelevant = true;
		UnLive2DActor->NetUpdateFrequency = 0.1f;
	}

	UnLive2DActor->GetUnLive2DRendererComponent()->RegisterComponent();
}

#if ENGINE_MAJOR_VERSION == 4 
void UActorFactoryUnLive2D::PostCreateBlueprint(UObject* Asset, AActor* CDO)
{
	if (Asset == NULL || CDO == NULL) return;

	UUnLive2D* UnLive2D = CastChecked<UUnLive2D>(Asset);
	AUnLive2DActor* UnLive2DActor = CastChecked<AUnLive2DActor>(CDO);
	UnLive2DActor->GetUnLive2DRendererComponent()->SetUnLive2D(UnLive2D);
}
#endif

bool UActorFactoryUnLive2D::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (!AssetData.IsValid() || !AssetData.GetClass()->IsChildOf(UUnLive2D::StaticClass()))
	{
		OutErrorMsg = NSLOCTEXT("CanCreateActor", "NoUnLive2D", "不是UnLive2D类型");
		return false;
	}
	return true;
}

UObject* UActorFactoryUnLive2D::GetAssetFromActorInstance(AActor* ActorInstance)
{
	check(ActorInstance->IsA(NewActorClass));
	AUnLive2DActor* NewActor = CastChecked<AUnLive2DActor>(ActorInstance);
	if (NewActor->GetUnLive2DRendererComponent())
	{
		return NewActor->GetUnLive2DRendererComponent()->GetUnLive2D();
	}
	
	return nullptr;
}

