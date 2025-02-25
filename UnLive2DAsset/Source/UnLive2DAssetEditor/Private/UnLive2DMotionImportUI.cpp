#include "UnLive2DMotionImportUI.h"


UUnLive2DMotionImportUI::UUnLive2DMotionImportUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FadeInTime = 1.f;
	FadeOutTime = 1.f;
	this->SetFlags(RF_Transactional);
}

void UUnLive2DMotionImportUI::ResetToDefault()
{
	ReloadConfig();
}

