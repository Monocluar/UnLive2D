
#pragma once

#include "CoreMinimal.h"
#include "UnLive2DAnimBlueprint.h"

struct UNLIVE2DASSET_API FUnLive2DAnimInstance
{
public:
	/** 创建良好的唯一类型哈希的静态帮助器 */
	static uint32 TypeHashCounter;

	// 动画资源
	class UUnLive2DMotion* MotionData;

	// 激活的动画蓝图驱动器
	struct FActiveUnLive2DAnimBlueprint* ActiveUnLive2DAnimBlueprint;

public:
	// 是否播放完成
	uint32 bIsFinished:1;

	/** 缓存哈希类型 */
	uint32 TypeHash;

	// 起始时间
	float StartTime;

public:

	FUnLive2DAnimInstance(const UPTRINT InAnimInstanceHash, struct FActiveUnLive2DAnimBlueprint& InActiveUnLive2DAnimBlueprint);

public:
	// 是否可以播放
	bool IsPlaying() const;
};

struct UNLIVE2DASSET_API FUnLive2DAnimParseParameters
{
public:
	// 从声音开始到开始的时间偏移
	float StartTime;

	// 声音是否应无缝循环
	uint8 bLooping : 1;

	// 是否暂停
	uint8 bIsPaused:1;

	// 是否可以重新触发
	uint8 bEnableRetrigger : 1;

public:
	FUnLive2DAnimParseParameters()
		: StartTime(-1.f)
		, bLooping(false)
		, bIsPaused(false)
		, bEnableRetrigger(false)
	{}
};


struct UNLIVE2DASSET_API FActiveUnLive2DAnimBlueprint
{
public:
	FActiveUnLive2DAnimBlueprint();

	virtual ~FActiveUnLive2DAnimBlueprint();


private:
	uint32 WorldID;
	TWeakObjectPtr<UUnLive2DAnimBlueprint> UnLive2DAnimBlueprint;

public:

	// 播放持续时间
	float PlaybackTimeNonVirtualized;

	// 请求的开始时间
	float RequestedStartTime;

	// 活动声音是否暂停,独立设置vs全局暂停或取消暂停。
	uint8 bIsPaused:1;

	// 是否播放完成
	uint8 bFinished:1;

	// 动画是否已虚拟化并实现
	uint8 bHasVirtualized:1; 

public:

	TMap<UPTRINT, uint32> UnLive2DAnimNodeOffsetMap;
	TArray<uint8> UnLive2DAnimNodeData;

private:

	TMap<UPTRINT, FUnLive2DAnimInstance*> UnLive2DAnimInstances;

public:
	virtual uint32 GetObjectId() const;

	// 更新动画实例
	void UpDataUnLive2DAnimBlueprintInstances(const float DeltaTime);

	friend FArchive& operator<<(FArchive& Ar, FActiveUnLive2DAnimBlueprint* ActiveUnLive2DAnimBlueprint);

	FUnLive2DAnimInstance* FindUnLive2DAnimInstance(const UPTRINT AnimInstanceHash);

	void RemoveAnimInstance(const UPTRINT AnimInstanceHash);

	FUnLive2DAnimInstance& AddAnimInstance(const UPTRINT AnimInstanceHash);

	const TMap<UPTRINT, FUnLive2DAnimInstance*>& GetAnimInstances() const
	{
		return UnLive2DAnimInstances;
	}

};