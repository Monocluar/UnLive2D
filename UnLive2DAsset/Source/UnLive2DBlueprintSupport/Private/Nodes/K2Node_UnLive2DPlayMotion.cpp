#include "Nodes/K2Node_UnLive2DPlayMotion.h"
#include "Animation/UnLive2DMotionPlayCallbackProxy.h"

#define LOCTEXT_NAMESPACE "K2Node"

UK2Node_UnLive2DRendererComponentPlayMotion::UK2Node_UnLive2DRendererComponentPlayMotion(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UUnLive2DMotionPlayCallbackProxy, CreateProxyObjectForPlayMotion_RendererComponent);
	ProxyFactoryClass = UUnLive2DMotionPlayCallbackProxy::StaticClass();
	ProxyClass = UUnLive2DMotionPlayCallbackProxy::StaticClass();
}

FText UK2Node_UnLive2DRendererComponentPlayMotion::GetTooltipText() const
{
	return LOCTEXT("K2Node_UnLive2DRendererComponent_Tooltip", "Plays a Motion on a UnLive2DRendererComponent");
}

FText UK2Node_UnLive2DRendererComponentPlayMotion::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("UnLive2DRendererComponentPlayMotion_NodeTitle", "Play Motion");
}

FText UK2Node_UnLive2DRendererComponentPlayMotion::GetMenuCategory() const
{
	return LOCTEXT("UnLive2DRendererComponentPlayMotion_MotionCategory", "Component|Motion");
}

UK2Node_UnLive2DViewRendererUIPlayMotion::UK2Node_UnLive2DViewRendererUIPlayMotion(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UUnLive2DMotionPlayCallbackProxy, CreateProxyObjectForPlayMotion_ViewRendererUI);
	ProxyFactoryClass = UUnLive2DMotionPlayCallbackProxy::StaticClass();
	ProxyClass = UUnLive2DMotionPlayCallbackProxy::StaticClass();
}

FText UK2Node_UnLive2DViewRendererUIPlayMotion::GetTooltipText() const
{
	return LOCTEXT("K2Node_UnLive2DViewRendererUI_Tooltip", "Plays a Motion on a UnLive2DViewRendererUI");
}


FText UK2Node_UnLive2DViewRendererUIPlayMotion::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("UnLive2DViewRendererUIPlayMotion_NodeTitle", "Play Motion");
}

FText UK2Node_UnLive2DViewRendererUIPlayMotion::GetMenuCategory() const
{
	return LOCTEXT("UnLive2DViewRendererUIPlayMotion_MotionCategory", "Component|Motion");
}

#undef LOCTEXT_NAMESPACE