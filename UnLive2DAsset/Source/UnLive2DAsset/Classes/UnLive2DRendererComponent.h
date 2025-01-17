
#pragma once

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#if WITH_EDITOR
#include "CubismBpLib.h"
#endif // WITH_EDITOR
#include "CubismConfig.h"

#include "UnLive2DRendererComponent.generated.h"

class UUnLive2D;
class UUnLive2DMotion;
class UMaterialInterface;
class FUnLive2DRawModel;



UCLASS(ClassGroup = UnLive2D, meta = (BlueprintSpawnableComponent), hidecategories=(Material,Mesh))
class UNLIVE2DASSET_API UUnLive2DRendererComponent : public UMeshComponent
{
	GENERATED_UCLASS_BODY()

protected:

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

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
	virtual class UBodySetup* GetBodySetup() override;
	//~ End UPrimitiveComponent Interface.


	virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials = false) const override;

protected:

	// 初始化
	virtual void InitUnLive2D();

protected:
	// UnLive2D资源
	UPROPERTY(Category = Live2D, EditAnywhere)
		UUnLive2D* SourceUnLive2D;


public:
	// Live2D颜色混合模式为CubismBlendMode_Normal使用的材质
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Rendering, meta = (AllowedClasses = "UMaterialInterface"))
	FSoftObjectPath UnLive2DNormalMaterial;

	// Live2D颜色混合模式为CubismBlendMode_Additive使用的材质
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Rendering, meta = (AllowedClasses = "UMaterialInterface"))
	FSoftObjectPath UnLive2DAdditiveMaterial;

	// Live2D颜色混合模式为CubismBlendMode_Multiplicative使用的材质
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Rendering, meta = (AllowedClasses = "UMaterialInterface"))
	FSoftObjectPath UnLive2DMultiplyMaterial;

	// 渲染模式
	UPROPERTY(EditAnywhere)
	EUnLive2DRenderType UnLive2DRenderType;

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
#endif

protected:

	/** 动画播放完成 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Anim")
		void OnMotionPlayeEnd();

private:

	// Live2D模型设置模块
	TSharedPtr<FUnLive2DRawModel> UnLive2DRawModel;

	FBoxSphereBounds LocalBounds;
	TObjectPtr<class UBodySetup> ProcMeshBodySetup;

	class UnLive2DProxyBase* UnLive2DSceneProxy;

};