#include "UnLive2DMotion.h"


#if WITH_EDITOR

bool UUnLive2DMotion::LoadLive2DMotionData(const FString& ReadMotionPath, EUnLive2DMotionGroup InMotionGroupType, int32 InMotionCount)
{
	MotionGroupType = InMotionGroupType;
	MotionCount = InMotionCount;
	MotionByteData.Empty();

	TArray<uint8> ModelFile;
	const bool ReadSuc = FFileHelper::LoadFileToArray(ModelFile, *ReadMotionPath);

	MotionByteData = ModelFile;

	return ReadSuc;
}

#endif
