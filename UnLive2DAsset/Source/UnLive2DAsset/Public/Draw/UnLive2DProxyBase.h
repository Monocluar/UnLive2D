#pragma once
#include "PrimitiveSceneProxy.h"

class UUnLive2DRendererComponent;
class FUnLive2DRawModel;

class UnLive2DProxyBase : public FPrimitiveSceneProxy
{
public:

	UnLive2DProxyBase(UUnLive2DRendererComponent* InComponent);

protected:

	bool UpDataDrawableIndexList(TArray<uint16>& OutSortedDrawableIndexList);

public:
	virtual void OnUpData(){};
	virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials = false) const {};

protected:
	// Live2D模型设置模块
	TWeakPtr<FUnLive2DRawModel> UnLive2DRawModel;

#if ENGINE_MAJOR_VERSION < 5
	UUnLive2DRendererComponent* OwnerComponent;
#else
	TObjectPtr<UUnLive2DRendererComponent> OwnerComponent;
#endif

private:
	// 绘制Buffer数
	TArray<uint16> DrawableIndexList;
};