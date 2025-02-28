
#pragma once

#include "CoreMinimal.h"

#include "UnLive2DPhysicsInternal.generated.h"

// 物理运算的目标类型
UENUM(BlueprintType)
enum class EUnLive2DPhysicsTargetType : uint8
{
	// 应用于参数
	Parameter,
};

// 物理运算的输入类型
UENUM(BlueprintType)
enum class EUnLive2DPhysicsSource : uint8
{
	// 从X轴位置
	X,
	// 从Y轴位置
	Y,
	// 从角度
	Angle,
};

// 物理运算的参数信息
USTRUCT(BlueprintType)
struct FUnLive2DPhysicsParameter
{
	GENERATED_USTRUCT_BODY()

	// 参数标识
	UPROPERTY(VisibleAnywhere)
	FName Id;

	// 目标类型
	UPROPERTY(VisibleAnywhere)
	EUnLive2DPhysicsTargetType TargetType;
};

// 物理运算的输入信息
USTRUCT(BlueprintType)
struct UNLIVE2DASSET_API FUnLive2DPhysicsInput
{
	GENERATED_USTRUCT_BODY()

	// 输入源参数
	UPROPERTY(VisibleAnywhere)
	FUnLive2DPhysicsParameter Source;

	// 重量
	UPROPERTY(VisibleAnywhere)
	float Weight = 0.f;

	// 值是否已反转
	UPROPERTY(VisibleAnywhere)
	bool bReflect = false;

	// 输入类型
	UPROPERTY(VisibleAnywhere)
	EUnLive2DPhysicsSource Type = EUnLive2DPhysicsSource::X;
};

// 物理运算的输出信息
USTRUCT(BlueprintType)
struct UNLIVE2DASSET_API FUnLive2DPhysicsOutput
{
	GENERATED_USTRUCT_BODY()

	// 钟摆索引
	UPROPERTY(VisibleAnywhere)
	int32 VertexIndex = 0; 

	// 缩放角度
	UPROPERTY(VisibleAnywhere)
	float AngleScale = 0.f;

	// 重力
	UPROPERTY(VisibleAnywhere)
	float Weight = 0.f;

	// 值是否已反转
	UPROPERTY(VisibleAnywhere)
	bool bReflect = false;

	// 输出类型
	UPROPERTY(VisibleAnywhere)
	EUnLive2DPhysicsSource Type = EUnLive2DPhysicsSource::X;

	// 目标参数
	UPROPERTY(VisibleAnywhere)
	FUnLive2DPhysicsParameter Destination;
};

// 用于计算物理运算的物理点信息
USTRUCT(BlueprintType)
struct UNLIVE2DASSET_API FUnLive2DPhysicsParticle
{
	GENERATED_USTRUCT_BODY()
	
	// 易于移动
	UPROPERTY(VisibleAnywhere)
	float Mobility;

	// 延迟
	UPROPERTY(VisibleAnywhere)
	float Delay;

	// 加速度
	UPROPERTY(VisibleAnywhere)
	float Acceleration;

	// 距离
	UPROPERTY(VisibleAnywhere)
	float Radius;

	// 当前位置
	UPROPERTY(VisibleAnywhere)
	FVector2D Position;
};

// Live2D 物理运算物理点管理
USTRUCT(BlueprintType)
struct UNLIVE2DASSET_API FUnLive2DPhysicsSetting
{
	GENERATED_USTRUCT_BODY()

	// 标识ID
	UPROPERTY(VisibleAnywhere)
	FName ID;

	// 标识名称
	UPROPERTY(VisibleAnywhere)
	FName Name;

	// 物理运算输入列表
	UPROPERTY(VisibleAnywhere)
	TArray<FUnLive2DPhysicsInput> Inputs;

	// 物理运算输出列表
	UPROPERTY(VisibleAnywhere)
	TArray<FUnLive2DPhysicsOutput> Outputs;

	// 物理运算的物理点列表
	UPROPERTY(VisibleAnywhere)
	TArray<FUnLive2DPhysicsParticle> Particles;
};

USTRUCT(BlueprintType)
struct UNLIVE2DASSET_API FUnLive2DPhysicsData
{
	GENERATED_USTRUCT_BODY()

	// 帧率
	UPROPERTY(VisibleAnywhere, Category = "Asset")
	float Fps;

	// 版本
	UPROPERTY(VisibleAnywhere, Category = "Asset")
	int32 Version;

	// 重力方向
	UPROPERTY(EditAnywhere, Category = "Physics")
	FVector2D Gravity;

	// 风力方向
	UPROPERTY(EditAnywhere, Category = "Physics")
	FVector2D Wind;

	// 输入总数
	UPROPERTY(EditAnywhere, Category = "Physics")
	int32 TotalInput;

	// 输出总数
	UPROPERTY(EditAnywhere, Category = "Physics")
	int32 TotalOutput;

	// 物理运算的物理点数
	UPROPERTY(EditAnywhere, Category = "Physics")
	int32 VertexCount;

	// 物理运算的数据
	UPROPERTY(EditAnywhere, Category = "Physics")
	TArray<FUnLive2DPhysicsSetting> PhysicsSettings;
};
