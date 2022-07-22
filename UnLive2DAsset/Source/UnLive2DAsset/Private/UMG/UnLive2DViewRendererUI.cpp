#include "UnLive2DViewRendererUI.h"
#include "Slate/SUnLive2DViewUI.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "UnLive2D.h"
#include "Engine/World.h"


#define LOCTEXT_NAMESPACE "UnLive2D"

UUnLive2DViewRendererUI::UUnLive2DViewRendererUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> NormalMaterial(TEXT("/UnLive2DAsset/Slate/UnLive2DSlateThrough_Normal"));
	UnLive2DNormalMaterial = NormalMaterial.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> AdditiveMaterial(TEXT("/UnLive2DAsset/Slate/UnLive2DSlateThrough_Additive"));
	UnLive2DAdditiveMaterial = AdditiveMaterial.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MultiplyMaterial(TEXT("/UnLive2DAsset/Slate/UnLive2DSlateThrough_Multiply"));
	UnLive2DMultiplyMaterial = MultiplyMaterial.Object;

	TextureParameterName = FName(TEXT("UnLive2D"));
}

void UUnLive2DViewRendererUI::PlayMotion(UUnLive2DMotion* InMotion)
{
	if (MySlateWidget.IsValid())
	{
		MySlateWidget->PlayMotion(InMotion);
	}
}

void UUnLive2DViewRendererUI::PlayExpression(UUnLive2DExpression* InExpression)
{
	if (MySlateWidget.IsValid())
	{
		MySlateWidget->PlayExpression(InExpression);
	}
}

#if WITH_EDITOR
const FText UUnLive2DViewRendererUI::GetPaletteCategory()
{
	return LOCTEXT("UnLive2D", "UnLive2D");
}

#endif

#if WITH_ACCESSIBILITY
TSharedPtr<SWidget> UUnLive2DViewRendererUI::GetAccessibleWidget() const
{
	return MySlateWidget;
}
#endif

TSharedRef<SWidget> UUnLive2DViewRendererUI::RebuildWidget()
{
	return SAssignNew(MySlateWidget, SUnLive2DViewUI, this);
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
	if (MySlateWidget.IsValid())
	{
		MySlateWidget->ReleaseRenderStateData();
		MySlateWidget.Reset();
	}
}

#undef LOCTEXT_NAMESPACE