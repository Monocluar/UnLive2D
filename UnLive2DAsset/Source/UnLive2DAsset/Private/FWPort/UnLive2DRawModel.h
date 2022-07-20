#pragma once

#include "CoreMinimal.h"
#include "CubismFramework.hpp"
#include "Model/CubismUserModel.hpp"
#include "Motion/CubismMotionQueueManager.hpp"
#include "UObject/WeakObjectPtrTemplates.h"
#include "UnLive2D.h"

using namespace Live2D::Cubism::Framework;
class UUnLive2DMotion;
class UUnLive2DExpression;

// Live2D 模型基础类
class FUnLive2DRawModel : public CubismUserModel
{
public:
    FUnLive2DRawModel(const class UUnLive2D* Owner);
    virtual ~FUnLive2DRawModel();

private:
    FUnLive2DRawModel(){}

public:

    // 加载资源
    bool LoadAsset(const struct FUnLive2DLoadData& InData);

    // 更新
    void OnUpDate(float InDeltaTime);

public:
	FORCEINLINE TWeakPtr<Csm::ICubismModelSetting> GetModelSetting() const { return Live2DModelSetting; };

#if WITH_EDITOR
	static FUnLive2DLoadData LoadLive2DFileDataFormPath(const FString& InPath, TArray<FString>& LoadTexturePaths, TArray<struct FUnLive2DMotionData>& LoadMotionData, TMap<FString, FUnLiveByteData>& LoadExpressionData);
#endif

    // 获取Parameter组
    const char** GetLive2DModelParameterIds();

    const float* GetLive2DModelParameterValues();

    FORCEINLINE const TWeakObjectPtr<const UUnLive2D> GetOwnerLive2D() const { return OwnerLive2D; };

public:
	// 拍打动画
	bool OnTapMotion(const FVector2D& InTapMotion);

	// 脸部朝向设置
	void SetDragPos(const FVector2D& InDragMotion);

    // 设置重力方向
    void SetPhysicsGravity(const FVector2D& InGravity);

    // 设置风力方向
	void SetPhysicsWind(const FVector2D& InWind);

    // 播放Live2D动作
	float StartMotion(UUnLive2DMotion* InMotion);

    // 播放Live2D表情
    float StartExpressions(UUnLive2DExpression* InExpressions);

private:

    // 释放动画系统
    void ReleaseMotions();

    // 释放表情系统
    void ReleaseExpressions();

    // 是否点击为该身体部位
	bool HitTest(const Csm::csmChar* HitAreaName, const FVector2D& InPos);


private:
    // 播放Live2D动作
    Csm::CubismMotionQueueEntryHandle StartMotion(const Csm::csmChar* Group, Csm::csmInt32 No, Csm::csmInt32 Priority);

    // 开始随机播放Live2D动作
	Csm::CubismMotionQueueEntryHandle StartRandomMotion(const Csm::csmChar* Group, Csm::csmInt32 Priority);

    // 设置随机播放表情系统
    void SetRandomExpression();

    // 设置播放表情系统
    void SetExpression(const uint32& ExpressionID);

private:

	const Csm::CubismId* ID_ParamAngleX; ///< ID: ParamAngleX
	const Csm::CubismId* ID_ParamAngleY; ///< ID: ParamAngleX
	const Csm::CubismId* ID_ParamAngleZ; ///< ID: ParamAngleX
	const Csm::CubismId* ID_ParamBodyAngleX; ///< ID: ParamBodyAngleX

	const Csm::CubismId* ID_ParamEyeBallX; ///< ID: ParamEyeBallX
	const Csm::CubismId* ID_ParamEyeBallY; ///< ID: ParamEyeBallXY

private:
    // Live2D 模型信息
    TSharedPtr<Csm::ICubismModelSetting> Live2DModelSetting;

    // 可播放的动画列表
    TMap<FName, Csm::ACubismMotion*> Live2DMotions;

    // 可播放的表情系统
    TMap<uint32, Csm::ACubismMotion*> Live2DExpressions;

    // 设置眨眼参数的ID
    Csm::csmVector<Csm::CubismIdHandle> EyeBlinkIds;

    // 设置口型参数的ID
    Csm::csmVector<Csm::CubismIdHandle> LipSyncIds;

private:

    TSharedPtr<CubismPhysics::Options> PhysicsData;

    TWeakObjectPtr<const UUnLive2D> OwnerLive2D;

#if WITH_EDITOR
private:
	// 运行时间
	float UserTimeSeconds;
#endif

public:
	FSimpleDelegate OnMotionPlayEnd;
};