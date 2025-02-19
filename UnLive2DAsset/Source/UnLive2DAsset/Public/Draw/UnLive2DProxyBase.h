#pragma once
#include "PrimitiveSceneProxy.h"
#include "IUnLive2DRenderBase.h"

class UUnLive2DRendererComponent;
class FUnLive2DRawModel;

class UnLive2DProxyBase : public FPrimitiveSceneProxy 
{
public:

	UnLive2DProxyBase(UUnLive2DRendererComponent* InComponent);

public:
	virtual bool OnUpData() = 0;
	virtual void OnUpDataRenderer(){};
	virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials = false) const {};
	virtual const FBoxSphereBounds& GetLocalBox() const;

#if WITH_EDITOR
	virtual void UpDataUnLive2DProperty(FName PropertyName){};
#endif

protected:

#if ENGINE_MAJOR_VERSION < 5
	UUnLive2DRendererComponent* OwnerComponent;
#else
	TObjectPtr<UUnLive2DRendererComponent> OwnerComponent;
#endif

private:
	// 绘制Buffer数
	TArray<uint16> DrawableIndexList;

protected:
	uint8 Live2DScale;
	FBoxSphereBounds LocalBox;
};