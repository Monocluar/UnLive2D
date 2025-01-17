#include "Draw/UnLive2DProxyBase.h"
#include "UnLive2DRendererComponent.h"
#include "FWPort/UnLive2DRawModel.h"

UnLive2DProxyBase::UnLive2DProxyBase(UUnLive2DRendererComponent* InComponent) 
	: FPrimitiveSceneProxy(InComponent)
	, UnLive2DRawModel(InComponent->GetUnLive2DRawModel())
	, OwnerComponent(InComponent)
{

}

bool UnLive2DProxyBase::UpDataDrawableIndexList(TArray<uint16>& OutSortedDrawableIndexList)
{
	if (!UnLive2DRawModel.IsValid()) return false;

	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel.Pin()->GetModel();
	if (UnLive2DModel == nullptr) return false;

	if (DrawableIndexList.Num() != 0)
	{
		bool bUpdataSections = false;
		for (const uint16& DrawableIndex : DrawableIndexList)
		{
			if (UnLive2DModel->GetDrawableDynamicFlagRenderOrderDidChange(DrawableIndex) || UnLive2DModel->GetDrawableDynamicFlagVisibilityDidChange(DrawableIndex) /*|| UnLive2DModel->GetDrawableDynamicFlagOpacityDidChange(DrawableIndex)*/)
			{
				bUpdataSections = true;
				break;
			}
		}
		if (!bUpdataSections) return false;
	}
	const Csm::csmInt32 DrawableCount = UnLive2DModel->GetDrawableCount();

	const Csm::csmInt32* RenderOrder = UnLive2DModel->GetDrawableRenderOrders();

	OutSortedDrawableIndexList.SetNum(DrawableCount);
	DrawableIndexList.SetNum(DrawableCount);

	for (csmInt32 i = 0; i < DrawableCount; i++)
	{
		const csmInt32 Order = RenderOrder[i];
		OutSortedDrawableIndexList[Order] = i;
		DrawableIndexList[Order] = i;
	}
	return true;
}

