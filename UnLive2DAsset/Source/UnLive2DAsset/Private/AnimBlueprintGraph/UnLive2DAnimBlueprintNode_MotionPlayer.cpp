#include "AnimBlueprintGraph/UnLive2DAnimBlueprintNode_MotionPlayer.h"
#include "UObject/FrameworkObjectVersion.h"
#include "UnLive2DAssetModule.h"
#include "Animation/ActiveUnLive2DAnimBlueprint.h"

#define LOCTEXT_NAMESPACE "UnLive2D"


UUnLive2DAnimBlueprintNode_MotionPlayer::UUnLive2DAnimBlueprintNode_MotionPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayRate = 1.f;
	bLooping = true;
}

void UUnLive2DAnimBlueprintNode_MotionPlayer::ParseNodes(FActiveUnLive2DAnimBlueprint& ActiveLive2DAnim, FUnLive2DAnimParseParameters& ParseParams, const UPTRINT NodeAnimInstanceHash)
{
	if (bAsyncLoading)
	{
		UE_LOG(LogUnLive2D, Verbose, TEXT("UnLive2DAnimBlueprintNode_MotionPlayer::ParseNodes 中%s的异步加载未完成，稍后将尝试播放"), *GetFullNameSafe(this));

		// 是否加载完
		ActiveLive2DAnim.bFinished = false;
		return;
	}

	if (UnLive2DMotion)
	{
		bool bAnimIsLoop = UnLive2DMotion->bLooping;
		UnLive2DMotion->bLooping = false;

		if (bLooping)
		{
			FUnLive2DAnimParseParameters UpdatedParams = ParseParams;
			UpdatedParams.bLooping = true;
			UnLive2DMotion->Parse(ActiveLive2DAnim, ParseParams, NodeAnimInstanceHash);
		}
		else
		{
			if (ParseParams.bEnableRetrigger)
			{
				UnLive2DMotion->Parse(ActiveLive2DAnim, ParseParams, NodeAnimInstanceHash);
			}
			else
			{
				if (!ActiveLive2DAnim.bHasVirtualized)
				{
					RETRIEVE_UNLIVE2DANIMBLUEPRINT_PAYLOAD(sizeof(int32));
					DECLARE_UNLIVE2DANIMBLUEPRINT_ELEMENT(int32, bPlayFailed);
					if (*RequiresInitialization)
					{
						bPlayFailed = 0;
					}

					const int32 InitActiveUnLive2DAnimInstanceNum = ActiveLive2DAnim.GetAnimInstances().Num();


					if (!bPlayFailed)
					{
						UnLive2DMotion->Parse(ActiveLive2DAnim, ParseParams, NodeAnimInstanceHash);
					}

					if (*RequiresInitialization != 0)
					{
						if (InitActiveUnLive2DAnimInstanceNum == ActiveLive2DAnim.GetAnimInstances().Num())
						{
							bPlayFailed = 1;
						}
						*RequiresInitialization = 0;
					}
				}
			}

		}

		UnLive2DMotion->bLooping = bAnimIsLoop;
	}
	else if (!UnLive2DMotionAssetPtr.IsNull() && !bAssetLoadRequestPending)
	{
		UE_LOG(LogUnLive2D, Warning, TEXT("尝试播放时，UUnLive2DAnimBlueprintNode_MotionPlayer::ParseNodes中需要加载%s，可能是因为质量节点已更改。"), *GetFullNameSafe(this));

		LoadAsset();
		bAssetLoadRequestPending = true;

	}
}

void UUnLive2DAnimBlueprintNode_MotionPlayer::SetUnLive2DMotion(UUnLive2DMotion* NewUnLive2DMotion)
{
	UnLive2DMotion = NewUnLive2DMotion;
	UnLive2DMotionAssetPtr = NewUnLive2DMotion;
}

void UUnLive2DAnimBlueprintNode_MotionPlayer::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);


	Ar.UsingCustomVersion(FFrameworkObjectVersion::GUID);

	if (Ar.CustomVer(FFrameworkObjectVersion::GUID) >= FFrameworkObjectVersion::HardSoundReferences)
	{
		if (Ar.IsLoading())
		{
			Ar << UnLive2DMotion;
		}
		else if (Ar.IsSaving())
		{
			UUnLive2DMotion* HardReference = (ShouldHardReferenceAsset(Ar.CookingTarget()) ? UnLive2DMotion : nullptr);
			Ar << HardReference;
		}
	}
}

#if WITH_EDITOR
void UUnLive2DAnimBlueprintNode_MotionPlayer::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	LoadAsset();
}

FText UUnLive2DAnimBlueprintNode_MotionPlayer::GetTitle() const
{
	FText MotionName;
	if (UnLive2DMotion)
	{
		MotionName = FText::FromName(UnLive2DMotion->GetFName());
	}
	else if (UnLive2DMotionAssetPtr.IsValid())
	{
		MotionName = FText::FromString(UnLive2DMotionAssetPtr.GetAssetName());
	}
	else
	{
		MotionName = LOCTEXT("NoMotion", "NONE");
	}

	FText Title;

	FFormatNamedArguments Arguments;
	Arguments.Add(TEXT("Description"), Super::GetTitle());
	Arguments.Add(TEXT("MotionName"), MotionName);
	if (bLooping)
	{
		Title = FText::Format(LOCTEXT("LoopingMotionDescription", "Looping {Description} : {MotionName}"), Arguments);
	}
	else
	{
		Title = FText::Format(LOCTEXT("NonLoopingMotionDescription", "{Description} : {MotionName}"), Arguments);
	}

	return Title;
}

#endif

int32 UUnLive2DAnimBlueprintNode_MotionPlayer::GetMaxChildNodes() const
{
	return 0;
}

float UUnLive2DAnimBlueprintNode_MotionPlayer::GetDuration()
{
	float Duartion = 0.f;
	if (UnLive2DMotion)
	{
		if (bLooping)
		{
			const float MontionLoopPlayerDuration = 10000.0f;
			Duartion = MontionLoopPlayerDuration;
		}
		else
		{
			Duartion = UnLive2DMotion->Duration;
		}
	}
	return Duartion;
}

void UUnLive2DAnimBlueprintNode_MotionPlayer::LoadAsset(bool bAddToRoot /*= false*/)
{
	if (IsAsyncLoading())
	{
		UnLive2DMotion = UnLive2DMotionAssetPtr.Get();
		if (UnLive2DMotion && UnLive2DMotion->HasAnyFlags(RF_NeedLoad)) // 该对象需要加载
		{
			UnLive2DMotion = nullptr;
		}

		if (UnLive2DMotion == nullptr)
		{
			const FString LongPackageName = UnLive2DMotionAssetPtr.GetLongPackageName();
			if (LongPackageName.IsEmpty()) return;
			bAsyncLoading = true;
			LoadPackageAsync(LongPackageName, FLoadPackageAsyncDelegate::CreateUObject(this, &UUnLive2DAnimBlueprintNode_MotionPlayer::OnMotionAssetLoaded, bAddToRoot));
		}
		else if (bAddToRoot)
		{
			UnLive2DMotion->AddToRoot();
		}

		if (UnLive2DMotion)
		{
			UnLive2DMotion->AddToCluster(this, true);
		}
	}
	else
	{
		UnLive2DMotion = UnLive2DMotionAssetPtr.LoadSynchronous();
		if (UnLive2DMotion)
		{
			if (bAddToRoot)
			{
				UnLive2DMotion->AddToRoot();
			}
			UnLive2DMotion->AddToCluster(this);
		}
	}
}

void UUnLive2DAnimBlueprintNode_MotionPlayer::ClearAssetReferences()
{
	UnLive2DMotion = nullptr;
}

void UUnLive2DAnimBlueprintNode_MotionPlayer::OnMotionAssetLoaded(const FName& PackageName, UPackage* Package, EAsyncLoadingResult::Type Result, bool bAddToRoot)
{
	if (Result == EAsyncLoadingResult::Succeeded)
	{
		UnLive2DMotion = UnLive2DMotionAssetPtr.Get();
		if (UnLive2DMotion)
		{
			if (bAddToRoot)
			{
				UnLive2DMotion->AddToRoot();
			}
			UnLive2DMotion->AddToCluster(this);
		}
	}
	bAsyncLoading = false;
}

#undef LOCTEXT_NAMESPACE