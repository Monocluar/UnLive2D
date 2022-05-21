#include "UnLive2DMotion.h"
#include "Misc/FileHelper.h"

#if WITH_EDITOR
#include "Motion/CubismMotion.hpp"
#endif


#if WITH_EDITOR

using namespace Live2D::Cubism::Framework;

bool UUnLive2DMotion::LoadLive2DMotionData(const FString& ReadMotionPath, EUnLive2DMotionGroup InMotionGroupType, int32 InMotionCount, float FadeInTime, float FadeOutTime)
{

	const bool ReadSuc = FFileHelper::LoadFileToArray(MotionData.MotionByteData, *ReadMotionPath);
	MotionData.FadeInTime = FadeInTime;
	MotionData.FadeOutTime = FadeOutTime;
	MotionData.MotionCount = InMotionCount;
	MotionData.MotionGroupType = InMotionGroupType;

	CubismMotion* Motion = CubismMotion::Create(MotionData.MotionByteData.GetData(), MotionData.MotionByteData.Num(), NULL); // 解析动画Json数据
	Duration = Motion->GetDuration();

	ACubismMotion::Delete(Motion);

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

