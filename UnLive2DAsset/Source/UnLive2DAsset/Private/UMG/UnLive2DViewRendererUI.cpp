#include "UnLive2DViewRendererUI.h"
#include "Slate/SUnLive2DViewUI.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "UnLive2D.h"
#include "Engine/World.h"
#include "UnLive2DSetting.h"
#include "Components/Widget.h"


#define LOCTEXT_NAMESPACE "UnLive2D"

UUnLive2DViewRendererUI::UUnLive2DViewRendererUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, PlayRate(1.f)
	, RenderTargetSize(1024)
{
	const UUnLive2DSetting* Setting = GetDefault<UUnLive2DSetting>();
	UnLive2DNormalMaterial = Setting->DefaultUnLive2DNormalMaterial_UI;

	UnLive2DAdditiveMaterial = Setting->DefaultUnLive2DAdditiveMaterial_UI;

	UnLive2DMultiplyMaterial = Setting->DefaultUnLive2DMultiplyMaterial_UI;

	UnLive2DRTMaterial = Setting->DefaultUnLive2DRenderTargetMaterial;
}

void UUnLive2DViewRendererUI::PlayMotion(UUnLive2DMotion* InMotion)
{
	if (MySlateWidget.IsValid())
	{
		//MySlateWidget->PlayMotion(InMotion);
	}
}

void UUnLive2DViewRendererUI::PlayExpression(UUnLive2DExpression* InExpression)
{
	if (MySlateWidget.IsValid())
	{
		//MySlateWidget->PlayExpression(InExpression);
	}
}

void UUnLive2DViewRendererUI::StopMotion()
{
	if (MySlateWidget.IsValid())
	{
		//MySlateWidget->StopMotion();
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
	return SAssignNew(MySlateWidget, SUnLive2DViewUI, GetUnLive2D())
			.RenderTargetSize(RenderTargetSize)
			.AdditiveMaterial(UnLive2DAdditiveMaterial)
			.MultiplyMaterial(UnLive2DMultiplyMaterial)
			.NormalMaterial(UnLive2DNormalMaterial)
			.RTMaterial(UnLive2DRTMaterial)
			.UnLive2DRenderType(EUnLive2DRenderType::Mesh);
}


void UUnLive2DViewRendererUI::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	if (MySlateWidget.IsValid())
	{
		TAttribute<float> PlayRateAttribute = PROPERTY_BINDING(float, PlayRate);

		MySlateWidget->SetUnLive2D(SourceUnLive2D);
		MySlateWidget->SetPlayRate(PlayRateAttribute);
		MySlateWidget->SetLive2DRenderType(UnLive2DRenderType);
	}
}

void UUnLive2DViewRendererUI::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	if (MySlateWidget.IsValid())
	{
		//MySlateWidget->ReleaseRenderStateData();
		MySlateWidget.Reset();
	}
}

#undef LOCTEXT_NAMESPACE