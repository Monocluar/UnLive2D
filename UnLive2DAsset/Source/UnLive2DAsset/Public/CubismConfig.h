#pragma once

#include "CoreMinimal.h"
#include "CubismConfig.generated.h"

/**
 * 模型渲染配置
 */
USTRUCT(BlueprintType)
struct FModelConfig
{
	GENERATED_USTRUCT_BODY()

public:
	FModelConfig(){};

public:

	/**
	 * 如果可能，我们将尝试在单个纹理中渲染所有遮罩
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bTryLowPreciseMask = true;

};


UENUM()
namespace EUnLive2DCollisionMode
{
	enum Type
	{
		/** Should this have no collison and not participate in physics? */
		None,

		Use2DPhysics UMETA(Hidden, DisplayName = "Use 2D Physics (Deprecated)"),

		/** Should this have 3D collision geometry and participate in the 3D physics world? */
		Use3DPhysics UMETA(DisplayName = "Use 3D Physics")
	};
}

USTRUCT()
struct FUnLive2DGeometryCollection
{
	GENERATED_USTRUCT_BODY()

public:
	FUnLive2DGeometryCollection()
		: BoxSize(ForceInitToZero)
		, BoxPosition(ForceInitToZero)
		, Rotation(0.0f)

	{
	}

public:
	// 多边形的顶点
	UPROPERTY(Category = Physics, EditAnywhere)
		TArray<FVector2D> Vertices;

	// <Box碰撞大小
	UPROPERTY(Category = Physics, VisibleAnywhere)
		FVector2D BoxSize;

	// <Box中心点
	UPROPERTY(Category = Physics, VisibleAnywhere)
		FVector2D BoxPosition;

	// 角度
	UPROPERTY(Category = Physics, VisibleAnywhere)
		float Rotation;

public:

	FVector2D ConvertTextureSpaceToShapeSpace(const FVector2D& TextureSpacePoint) const
	{
		return (TextureSpacePoint - BoxPosition).GetRotated(Rotation);
	}

	FVector2D ConvertShapeSpaceToTextureSpace(const FVector2D& ShapeSpacePoint) const
	{
		return ShapeSpacePoint.GetRotated(-Rotation) + BoxPosition;
	}

	// 返回纹理空间中的多边形质心
	FVector2D GetPolygonCentroid() const
	{
		FBox2D Bounds(ForceInit);

		FVector2D LocalSpaceResult = FVector2D::ZeroVector;

		for (const FVector2D& Vertex : Vertices)
		{
			Bounds += Vertex;
		}

		if (Vertices.Num() > 0)
		{
			LocalSpaceResult = Bounds.GetCenter();
		}

		return ConvertShapeSpaceToTextureSpace(LocalSpaceResult);
	}

	void GetTextureSpaceVertices(TArray<FVector2D>& InOutVertices) const
	{
		InOutVertices.Reserve(InOutVertices.Num() + Vertices.Num());
		for (const FVector2D& Vertex : Vertices)
		{
			InOutVertices.Add(ConvertShapeSpaceToTextureSpace(Vertex));
		}
	}

	// 设置新轴心并调整所有顶点以使其相对于轴心
	void SetNewPivot(const FVector2D& NewPosInTextureSpace)
	{
		const FVector2D DeltaTexturePos = BoxPosition - NewPosInTextureSpace;
		for (FVector2D& Vertex : Vertices)
		{
			const FVector2D NewVertexPosTS = ConvertShapeSpaceToTextureSpace(Vertex) + DeltaTexturePos;
			Vertex = ConvertTextureSpaceToShapeSpace(NewVertexPosTS);
		}

		BoxPosition = NewPosInTextureSpace;
	}
};

USTRUCT()
struct FUnLiveByteData
{
	GENERATED_USTRUCT_BODY()

public:
	FUnLiveByteData()
	{}

	FUnLiveByteData(TArray<uint8>& Temp)
		: ByteData(MoveTemp(Temp))
	{}

public:
	UPROPERTY()
		TArray<uint8> ByteData;
};

USTRUCT()
struct FUnLive2DLoadData
{
	GENERATED_USTRUCT_BODY()

public:
	// <Live2D模型数据
	UPROPERTY()
		TArray<uint8> Live2DModelData;

	// <Live2D立体数据
	UPROPERTY()
		TArray<uint8> Live2DCubismData;

	// <Live2D表情数据
	UPROPERTY()
		TMap<FName, FUnLiveByteData> Live2DExpressionData;

	// <Live2D物理数据
	UPROPERTY()
		TArray<uint8> Live2DPhysicsData;

	// <Live2DPose数据
	UPROPERTY()
		TArray<uint8> Live2DPoseData;

	// <Live2D用户数据
	UPROPERTY()
		TArray<uint8> Live2DUserDataData;

	// <Live2D贴图数据
	UPROPERTY()
		TArray<FUnLiveByteData> Live2DTexture2DData;
};