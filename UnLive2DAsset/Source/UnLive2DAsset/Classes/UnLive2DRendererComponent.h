
#pragma once

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#if WITH_EDITOR
#include "CubismBpLib.h"
#endif // WITH_EDITOR
#include "CubismConfig.h"
#include "Draw/IUnLive2DRenderBase.h"

#include "UnLive2DRendererComponent.generated.h"

class UUnLive2D;
class UUnLive2DMotion;
class UMaterialInterface;
class FUnLive2DRawModel;
class UTextureRenderTarget2D;


UCLASS(ClassGroup = UnLive2D, meta = (BlueprintSpawnableComponent), hidecategories = (Material, Mesh))
class UNLIVE2DASSET_API UUnLive2DRendererComponent : public UMeshComponent
{
	GENERATED_UCLASS_BODY()

protected:

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void SendRenderDynamicData_Concurrent() override;

	virtual void OnRegister() override;

	//~ Begin UObject Interface
	virtual void PostLoad() override;
	//~ End UObject Interface.
	// 
	//~ Begin USceneComponent Interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ Begin USceneComponent Interface.

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual FCollisionShape GetCollisionShape(float Inflation) const override;
	//~ End UPrimitiveComponent Interface.


	virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials = false) const override;
	virtual void BeginDestroy() override;

public:
	virtual class UBodySetup* GetBodySetup() override;

protected:

	// 初始化
	virtual void InitUnLive2D();

protected:
	// UnLive2D资源
	UPROPERTY(Category = Live2D, EditAnywhere)
		UUnLive2D* SourceUnLive2D;


	// 动画频率
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Live2D, meta = (ClampMin = 0.0f))
		float PlayRate;

public:
	// 渲染模式
	UPROPERTY(EditAnywhere, Category = Rendering)
		EUnLive2DRenderType UnLive2DRenderType;

public: // Mesh
	// Live2D颜色混合模式为CubismBlendMode_Normal使用的材质
	UPROPERTY(EditAnywhere, Category = Rendering, meta = (AllowedClasses = "MaterialInterface", EditCondition = "UnLive2DRenderType == EUnLive2DRenderType::Mesh", EditConditionHides))
		FSoftObjectPath UnLive2DNormalMaterial;

	// Live2D颜色混合模式为CubismBlendMode_Additive使用的材质
	UPROPERTY(EditAnywhere, Category = Rendering, meta = (AllowedClasses = "MaterialInterface", EditCondition = "UnLive2DRenderType == EUnLive2DRenderType::Mesh", EditConditionHides))
		FSoftObjectPath UnLive2DAdditiveMaterial;

	// Live2D颜色混合模式为CubismBlendMode_Multiplicative使用的材质
	UPROPERTY(EditAnywhere, Category = Rendering, meta = (AllowedClasses = "MaterialInterface", EditCondition = "UnLive2DRenderType == EUnLive2DRenderType::Mesh", EditConditionHides))
		FSoftObjectPath UnLive2DMultiplyMaterial;

public: // RenderTarget

	// 渲染大小
	UPROPERTY(EditAnywhere, Category = Rendering, meta = (UIMin = "256", UIMax = "2048", EditCondition = "UnLive2DRenderType == EUnLive2DRenderType::RenderTarget", EditConditionHides))
		int32 RenderTargetSize;

	UPROPERTY(EditAnywhere, Category = Rendering, meta = (AllowedClasses = "MaterialInterface", EditCondition = "UnLive2DRenderType == EUnLive2DRenderType::RenderTarget", EditConditionHides))
		FSoftObjectPath UnLive2DRTMaterial;

public:
	UFUNCTION(BlueprintCallable, Category = "Live2D")
		bool SetUnLive2D(UUnLive2D* NewUnLive2D);

	UFUNCTION(BlueprintCallable, Category = "Live2D")
		UUnLive2D* GetUnLive2D() const { return SourceUnLive2D; }

	// 播放动画
	//UFUNCTION(BlueprintCallable, Category = "Anim")
	virtual void  PlayMotion(UUnLive2DMotion* InMotion);

	// 播放表情
	UFUNCTION(BlueprintCallable, Category = "Anim")
		virtual void PlayExpression(UUnLive2DExpression* InExpression);

	// 停止播放动画
	UFUNCTION(BlueprintCallable, Category = "Anim")
		virtual void StopMotion();


	FORCEINLINE TWeakPtr<FUnLive2DRawModel> GetUnLive2DRawModel() const { return UnLive2DRawModel; }

#if WITH_EDITOR
	bool GetModelParamterGroup(TArray<FUnLive2DParameterData>& ParameterArr);

	void SetModelParamterValue(int32  ParameterID, float NewParameter, EUnLive2DExpressionBlendType::Type BlendType = EUnLive2DExpressionBlendType::ExpressionBlendType_Overwrite);

	bool GetModelParamterIDData(FName ParameterStr, FUnLive2DParameterData& Parameter);

	bool GetModelParamterID(FName ParameterStr, int32& ParameterID);

	void UpDataUnLive2DProperty(FName PropertyName);
#endif

public:

	void ClearRTCache();
	void InitLive2DRenderData(EUnLive2DRenderType InRenderType, int32 BufferHeight);

	FTextureRHIRef GetMaskTextureRHIRef() const;
	UTextureRenderTarget2D* GetTextureRenderTarget2D() const;

protected:

	/** 动画播放完成 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Anim")
		void OnMotionPlayeEnd();

private:

	// Live2D模型设置模块
	TSharedPtr<FUnLive2DRawModel> UnLive2DRawModel;

	class UBodySetup* ProcMeshBodySetup;

	FBoxSphereBounds LocalBounds;


private:
	TWeakObjectPtr<UTextureRenderTarget2D> MaskBufferRenderTarget; //遮罩渲染缓冲图片

	FTextureRHIRef MaskBuffer; // 遮罩图

};