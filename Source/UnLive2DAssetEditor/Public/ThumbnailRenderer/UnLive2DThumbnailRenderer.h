
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "ThumbnailRendering/DefaultSizedThumbnailRenderer.h"
#include "UnLive2D.h"
#include "UnLive2DThumbnailRenderer.generated.h"


class SUnLive2DViewUI;

UCLASS()
class UNLIVE2DASSETEDITOR_API UUnLive2DThumbnailRenderer : public UDefaultSizedThumbnailRenderer
{

	GENERATED_UCLASS_BODY()

protected:

	// UThumbnailRenderer interface
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget*, FCanvas* Canvas, bool bAdditionalViewFamily) override;
	// End of UThumbnailRenderer interface

	// UObject implementation
	virtual void BeginDestroy() override;

public:

	const UUnLive2D* GetUnLive2D() const;

	//TSharedRef<class FUnLive2DRenderState> InitUnLive2DRender();

	virtual void SlateUpDataRender(TWeakPtr<class FUnLive2DRawModel> InUnLive2DRawModel);

private:

	class FWidgetRenderer* WidgetRenderer;

	UUnLive2D* SourceUnLive2D;

	// UnLive2D 渲染模块
	TSharedPtr<class FUnLive2DRenderState> UnLive2DRenderPtr;


	TSharedPtr<SUnLive2DViewUI> MySlateWidget;
};