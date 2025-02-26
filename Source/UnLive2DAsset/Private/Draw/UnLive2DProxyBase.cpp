#include "Draw/UnLive2DProxyBase.h"
#include "UnLive2DRendererComponent.h"
#include "FWPort/UnLive2DRawModel.h"

UnLive2DProxyBase::UnLive2DProxyBase(UUnLive2DRendererComponent* InComponent) 
	: FPrimitiveSceneProxy(InComponent)
	, OwnerComponent(InComponent)
	, Live2DScale(InComponent->GetUnLive2D()->Live2DScale)
	, LocalBox(ForceInit)
{
}

const FBoxSphereBounds& UnLive2DProxyBase::GetLocalBox() const
{
	return LocalBox;
}

