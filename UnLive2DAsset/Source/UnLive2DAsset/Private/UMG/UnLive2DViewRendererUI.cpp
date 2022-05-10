#include "Classes/UnLive2DViewRendererUI.h"


#define LOCTEXT_NAMESPACE "UnLive2D"

UUnLive2DViewRendererUI::UUnLive2DViewRendererUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> NormalMaterial(TEXT("/UnLive2DAsset/UnLive2DPassNormalMaterial"));
	UnLive2DNormalMaterial = NormalMaterial.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> AdditiveMaterial(TEXT("/UnLive2DAsset/UnLive2DPassAdditiveMaterial"));
	UnLive2DAdditiveMaterial = AdditiveMaterial.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MultiplyMaterial(TEXT("/UnLive2DAsset/UnLive2DPassMultiplyMaterial"));
	UnLive2DMultiplyMaterial = MultiplyMaterial.Object;

	TextureParameterName = FName(TEXT("SpriteTexture"));
}

#if WITH_EDITOR
const FText UUnLive2DViewRendererUI::GetPaletteCategory()
{
	return LOCTEXT("UnLive2D", "UnLive2D");
}
#endif

TSharedRef<SWidget> UUnLive2DViewRendererUI::RebuildWidget()
{
	MySlateWidget = SNew(SUnLive2DViewUI)
			.OwnerWidget(this);
	return MySlateWidget.ToSharedRef();
}


void UUnLive2DViewRendererUI::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (MySlateWidget.IsValid())
	{
		MySlateWidget->SetUnLive2D(SourceUnLive2D);
	}
}

void UUnLive2DViewRendererUI::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	MySlateWidget.Reset();
}

#undef LOCTEXT_NAMESPACE