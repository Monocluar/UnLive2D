#include "Animation/ActiveUnLive2DAnimBlueprint.h"

uint32 FUnLive2DAnimInstance::TypeHashCounter = 0;

FActiveUnLive2DAnimBlueprint::FActiveUnLive2DAnimBlueprint()
	: PlaybackTimeNonVirtualized(0.f)
	, RequestedStartTime(0.f)
	, bIsPaused(false)
	, bFinished(false)
	, bHasVirtualized(false)
{

}

FActiveUnLive2DAnimBlueprint::~FActiveUnLive2DAnimBlueprint()
{

}

uint32 FActiveUnLive2DAnimBlueprint::GetObjectId() const
{
	if (!UnLive2DAnimBlueprint.IsValid()) return INDEX_NONE;

	return UnLive2DAnimBlueprint->GetUniqueID();
}

void FActiveUnLive2DAnimBlueprint::UpDataUnLive2DAnimBlueprintInstances(const float DeltaTime)
{
	if (!UnLive2DAnimBlueprint.IsValid() || !UnLive2DAnimBlueprint->IsPlayable()) return;

	bFinished = true;

	FUnLive2DAnimParseParameters ParseParams;
	ParseParams.StartTime = RequestedStartTime;
	ParseParams.bIsPaused = bIsPaused;

	UnLive2DAnimBlueprint->Parse(*this, ParseParams);

	PlaybackTimeNonVirtualized += DeltaTime;
}

FUnLive2DAnimInstance* FActiveUnLive2DAnimBlueprint::FindUnLive2DAnimInstance(const UPTRINT AnimInstanceHash)
{
	return UnLive2DAnimInstances.FindRef(AnimInstanceHash);
}

void FActiveUnLive2DAnimBlueprint::RemoveAnimInstance(const UPTRINT AnimInstanceHash)
{
	if (FUnLive2DAnimInstance* AnimInstance = UnLive2DAnimInstances.FindRef(AnimInstanceHash))
	{
		UnLive2DAnimInstances.Remove(AnimInstanceHash);
		delete AnimInstance;
	}
}

FUnLive2DAnimInstance& FActiveUnLive2DAnimBlueprint::AddAnimInstance(const UPTRINT AnimInstanceHash)
{
	FUnLive2DAnimInstance* AnimInstance = new FUnLive2DAnimInstance(AnimInstanceHash, *this);
	UnLive2DAnimInstances.Add(AnimInstanceHash, AnimInstance);
	return *AnimInstance;
}

FArchive& operator<<(FArchive& Ar, FActiveUnLive2DAnimBlueprint* ActiveUnLive2DAnimBlueprint)
{
	/*if (!Ar.IsLoading() && !Ar.IsSaving())
	{
		Ar << ActiveUnLive2DAnimBlueprint->UnLive2DAnimBlueprint;
		Ar << ActiveUnLive2DAnimBlueprint->UnLive2DAnimInstances;
		Ar << ActiveUnLive2DAnimBlueprint->UnLive2DAnimNodeOffsetMap;
	}*/
	return(Ar);
}

FUnLive2DAnimInstance::FUnLive2DAnimInstance(const UPTRINT InAnimInstanceHash, struct FActiveUnLive2DAnimBlueprint& InActiveUnLive2DAnimBlueprint)
	: MotionData(nullptr)
	, ActiveUnLive2DAnimBlueprint(&InActiveUnLive2DAnimBlueprint)
	, bIsFinished(false)
	, StartTime(-1.f)
{
	TypeHash =  ++TypeHashCounter;
}

bool FUnLive2DAnimInstance::IsPlaying() const
{
	if (MotionData == nullptr) return false;

	return true;
}
