#include "UnLive2DMotionImportUI.h"


UUnLive2DMotionImportUI::UUnLive2DMotionImportUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MotionCount = 0;
	MotionGroupType = EUnLive2DMotionGroup::None;
	this->SetFlags(RF_Transactional);
}

void UUnLive2DMotionImportUI::ResetToDefault()
{
	ReloadConfig();
}

