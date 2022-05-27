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

void UUnLive2DMotion::AddAssetUserData(UAssetUserData* InUserData)
{
	if (InUserData != nullptr)
	{
		UAssetUserData* ExistingData = GetAssetUserDataOfClass(InUserData->GetClass());
		if (ExistingData != nullptr)
		{
			AssetUserData.Remove(ExistingData);
		}
		AssetUserData.Add(InUserData);
	}
}

void UUnLive2DMotion::RemoveUserDataOfClass(TSubclassOf<UAssetUserData> InUserDataClass)
{
	for (int32 DataIdx = 0; DataIdx < AssetUserData.Num(); DataIdx++)
	{
		UAssetUserData* Datum = AssetUserData[DataIdx];
		if (Datum != nullptr && Datum->IsA(InUserDataClass))
		{
			AssetUserData.RemoveAt(DataIdx);
			return;
		}
	}
}

UAssetUserData* UUnLive2DMotion::GetAssetUserDataOfClass(TSubclassOf<UAssetUserData> InUserDataClass)
{
	for (int32 DataIdx = 0; DataIdx < AssetUserData.Num(); DataIdx++)
	{
		UAssetUserData* Datum = AssetUserData[DataIdx];
		if (Datum != nullptr && Datum->IsA(InUserDataClass))
		{
			return Datum;
		}
	}
	return nullptr;
}

const TArray<UAssetUserData*>* UUnLive2DMotion::GetAssetUserDataArray() const
{
	return &AssetUserData;
}

