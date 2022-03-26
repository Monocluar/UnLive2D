#include "UnLive2DMotion.h"
#include "Misc/FileHelper.h"


#if WITH_EDITOR

bool UUnLive2DMotion::LoadLive2DMotionData(const FString& ReadMotionPath, EUnLive2DMotionGroup InMotionGroupType, int32 InMotionCount, float FadeInTime, float FadeOutTime)
{

	const bool ReadSuc = FFileHelper::LoadFileToArray(MotionData.MotionByteData, *ReadMotionPath);
	MotionData.FadeInTime = FadeInTime;
	MotionData.FadeOutTime = FadeOutTime;
	MotionData.MotionCount = InMotionCount;
	MotionData.MotionGroupType = InMotionGroupType;

	return ReadSuc;
}

void UUnLive2DMotion::SetLive2DMotionData(FUnLive2DMotionData& InMotionData)
{
	MotionData = InMotionData;
}

#endif

const FUnLive2DMotionData* UUnLive2DMotion::GetMotionData()
{
	return &MotionData;
}

