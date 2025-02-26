#include "Animation/UnLive2DMotion.h"

#if WITH_EDITOR
#include "Misc/FileHelper.h"
#include "Motion/CubismMotion.hpp"
#endif
#include "Animation/ActiveUnLive2DAnimBlueprint.h"
#include "UnLive2DRendererComponent.h"

UUnLive2DMotion::UUnLive2DMotion()
{
	bFinished = true;
}


#if WITH_EDITOR
using namespace Live2D::Cubism::Framework;

bool UUnLive2DMotion::LoadLive2DMotionData(const FString& ReadMotionPath, float FadeInTime, float FadeOutTime)
{

	const bool ReadSuc = FFileHelper::LoadFileToArray(MotionData.MotionByteData, *ReadMotionPath);
	MotionData.FadeInTime = FadeInTime;
	MotionData.FadeOutTime = FadeOutTime;

	CubismMotion* Motion = CubismMotion::Create(MotionData.MotionByteData.GetData(), MotionData.MotionByteData.Num(), NULL); // 解析动画Json数据
	Duration = Motion->GetDuration();
	bLooping = Motion->IsLoop();

	ACubismMotion::Delete(Motion);

	return ReadSuc;
}

void UUnLive2DMotion::SetLive2DMotionData(FUnLive2DMotionData& InMotionData)
{
	MotionData = InMotionData;


	CubismMotion* Motion = CubismMotion::Create(MotionData.MotionByteData.GetData(), MotionData.MotionByteData.Num(), NULL); // 解析动画Json数据
	Duration = Motion->GetDuration();
	bLooping = Motion->IsLoop();

	ACubismMotion::Delete(Motion);
}

#endif

const FUnLive2DMotionData* UUnLive2DMotion::GetMotionData()
{
	return &MotionData;
}

void UUnLive2DMotion::OnPlayAnimEnd()
{
	bFinished = true;
	if (OnUnLive2DMotionEnded.IsBound())
	{
		OnUnLive2DMotionEnded.Execute(this, false);
	}
	
}

void UUnLive2DMotion::OnPlayAnimInterrupted()
{
	bFinished = true;
	if (OnUnLive2DMotionEnded.IsBound())
	{
		OnUnLive2DMotionEnded.Execute(this, true);
	}
}

void UUnLive2DMotion::Parse( FActiveUnLive2DAnimBlueprint& ActiveLive2DAnim, FUnLive2DAnimParseParameters& ParseParams, const UPTRINT NodeAnimInstanceHash)
{
	if (UnLive2D == nullptr) return;

	FUnLive2DAnimInstance* AnimInstance = ActiveLive2DAnim.FindUnLive2DAnimInstance(NodeAnimInstanceHash);

	const bool bIsNewAnim = AnimInstance == nullptr;

	if (!bIsNewAnim)
	{
		AnimInstance = &HandleStart(ActiveLive2DAnim, NodeAnimInstanceHash);
	}

	if (bLooping || ParseParams.bLooping)
	{
		AnimInstance->bIsFinished = false;
	}

	if (AnimInstance->bIsFinished)
	{
		return;
	}

	AnimInstance->StartTime = ParseParams.StartTime;

	bool bAlwaysPlay = false;

	if (AnimInstance->IsPlaying())
	{
		ActiveLive2DAnim.bFinished = false;
	}

	if (bFinished)
	{

	}
	else
	{

	}
}

struct FUnLive2DAnimInstance& UUnLive2DMotion::HandleStart(struct FActiveUnLive2DAnimBlueprint& ActiveLive2DAnim, const UPTRINT NodeAnimInstanceHash) const
{
	FUnLive2DAnimInstance& AnimInstance = ActiveLive2DAnim.AddAnimInstance(NodeAnimInstanceHash);

	return AnimInstance;
}

