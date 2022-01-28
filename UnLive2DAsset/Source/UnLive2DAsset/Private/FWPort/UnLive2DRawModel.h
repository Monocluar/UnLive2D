#pragma once

#include "CoreMinimal.h"

#include "CubismFramework.hpp"
#include "Model/CubismUserModel.hpp"
#include "Motion/CubismMotionQueueManager.hpp"

using namespace Live2D::Cubism::Framework;

// Live2D 模型基础类
class FUnLive2DRawModel : public Csm::CubismUserModel
{
public:
    FUnLive2DRawModel(class UUnLive2D* Owner);
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

    // 获取所有动画列表数据
    FORCEINLINE TMap<FName, TArray<FName>>& GetAllMotionGroup() { return AllMotionGroup; };

#if WITH_EDITOR
	static FUnLive2DLoadData LoadLive2DFileDataFormPath(const FString& InPath);
#endif

public:
    // 播放动画
    void PlayMotion(const FName& InName);

    // 获取动画优先级
    int32 GetPlayMotionPriority(const FName& InName);

    // 获取动画组名称
	FName GetPlayMotionGroupName(const FName& InName, int32& PriorityIndex);

	// 拍打动画
	bool OnTapMotion(const FVector2D& InTapMotion);

	// 脸部朝向设置
	void SetDragPos(const FVector2D& InDragMotion);

    // 设置重力方向
    void SetPhysicsGravity(const FVector2D& InGravity);

    // 设置风力方向
    void SetPhysicsWind(const FVector2D& InWind);

private:

    // 加载动画列表数据
    void PreloadMotionGroup(const Csm::csmChar* Group);

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
    void SetExpression(const FName& ExpressionID);

protected:

    // 立体模块加载
    FORCEINLINE void LoadCubismModel();

    // 表情模块加载
    FORCEINLINE void LoadExpressionModel();

    // 物理模块加载
    FORCEINLINE void LoadPhysicsModel();

    // 姿态模块加载
    FORCEINLINE void LoadPoseModel();

    // 角色数据加载
    FORCEINLINE void LoadUserDataModel();


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
    TMap<FName, Csm::ACubismMotion*> Live2DExpressions;

    // 设置眨眼参数的ID
    Csm::csmVector<Csm::CubismIdHandle> EyeBlinkIds;

    // 设置口型参数的ID
    Csm::csmVector<Csm::CubismIdHandle> LipSyncIds;

private:

    TMap<FName, TArray<FName>> AllMotionGroup;

    TSharedPtr<CubismPhysics::Options> PhysicsData;

    TWeakObjectPtr<class UUnLive2D> OwnerLive2D;

#if WITH_EDITOR
private:
	// 运行时间
	float UserTimeSeconds;
#endif

public:
	FSimpleDelegate OnMotionPlayEnd;
};