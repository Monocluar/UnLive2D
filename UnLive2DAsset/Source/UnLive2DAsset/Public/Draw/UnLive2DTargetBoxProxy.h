
#pragma once
#include "PrimitiveSceneProxy.h"
#include "UnLive2DProxyBase.h"
#include "RenderResource.h"

class UUnLive2DRendererComponent;
class UTextureRenderTarget2D;
class CubismClippingManager_UE;
class FUnLive2DRawModel;
class UMaterialInstanceDynamic;
class FRHICommandListImmediate;

namespace Live2D { namespace Cubism { namespace Framework 
{
	class CubismModel;
}}}

class FUnLive2DTargetBoxProxy final : public UnLive2DProxyBase
{
	enum EUnLive2DShaderFlags : uint8
	{
		BlendMode_Nor		= 1 << 0,
		BlendMode_Add		= 1 << 1,
		BlendMode_Mul		= 1 << 2,
		Mesk				= 1 << 3,
		InvertedMesk		= 1 << 4,
	};

	struct FUnLive2DSectionData
	{
		TArray<int32> DrawableCounts;
		uint8 Flags;
		uint8 TextureIndex;
		FBufferRHIRef CacheIndexBuffersRHI;
		FBufferRHIRef CacheVertexBufferRHI;
		uint16 CacheIndexCount;
		uint16 CacheVerticesIndexCount;

		~FUnLive2DSectionData();
	};

	struct FUnLive2DRenderTargetVertex
	{
		FVector2f Position;
		FVector2f UV;
		FLinearColor Color;
	};

	struct FUnLive2DRenderTargetVertex_Mask
	{
		FVector2f Position;
		FVector2f UV;
		FLinearColor Color;
		FVector4f ClipPosition; 
		FLinearColor ChanelFlag;
	};

public:
	class FUnLive2DTargetVertexDeclaration : public FRenderResource
	{
	public:

		FVertexDeclarationRHIRef VertexDeclarationRHI;

		virtual ~FUnLive2DTargetVertexDeclaration(){}

		virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
		virtual void ReleaseRHI() override;
	};

	class FUnLive2DTargetVertexDeclaration_Mask : public FRenderResource
	{
	public:

		FVertexDeclarationRHIRef VertexDeclarationRHI;

		virtual ~FUnLive2DTargetVertexDeclaration_Mask() {}

		virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
		virtual void ReleaseRHI() override;
	};
	

private:
	void UpdateSection_RenderThread(FRHICommandListImmediate& RHICmdList);

protected:
	virtual void OnUpData() override;
	virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials = false) const override;

	SIZE_T GetTypeHash() const override;
	virtual uint32 GetMemoryFootprint() const override;

	// FPrimitiveSceneProxy interface.
	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
	// End of FPrimitiveSceneProxy interface.

private:
	bool UpdataSections(FRHICommandListImmediate& RHICmdList);

	uint8 GetUnLive2DShaderFlagsByDrawableIndex(const uint16& InDrawableIndex, Live2D::Cubism::Framework::CubismModel* InUnLive2DModel) const;

	void DrawSeparateToRenderTarget_RenderThread(FRHICommandListImmediate& RHICmdList, FTextureRenderTargetResource* OutTextureRenderTargetResource);

	template<typename T>
	constexpr uint16 GetVertexBySectionData(FRHICommandListImmediate& RHICmdList, Live2D::Cubism::Framework::CubismModel* UnLive2DModel, FUnLive2DSectionData& InSectionData);

	const UTexture2D* GetTexture(const uint8& TextureIndex) const;

	bool IsCombinedbBatchDidChange(const Live2D::Cubism::Framework::CubismModel* UnLive2DModel,const FUnLive2DSectionData& InSectionData) const;

public:

	FUnLive2DTargetBoxProxy(UUnLive2DRendererComponent* InComponent);
	~FUnLive2DTargetBoxProxy();

private:
	// 裁剪管理器
	TUniquePtr<CubismClippingManager_UE> UnLive2DClippingManager;
	TObjectPtr<UTextureRenderTarget2D> RenderTarget; //渲染缓冲图片
	// 使用材质
	TObjectPtr<UMaterialInstanceDynamic> MaterialInstance;
	// 锚点
	FVector2D Pivot;

	// 是否合批
	bool bCombinedbBatch;
	// 当前绘制使用数据
	TArray<FUnLive2DSectionData> UnLive2DSectionDataArr;

	float BoundsScale;

	// 遮罩图
	FTextureRHIRef MaskBuffer;
};

