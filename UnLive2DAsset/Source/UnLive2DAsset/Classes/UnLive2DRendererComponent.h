
#pragma once

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#include "ProceduralMeshComponent.h"

#if WITH_EDITOR
#include "CubismBpLib.h"
#endif // WITH_EDITOR

#include "UnLive2DRendererComponent.generated.h"

class UUnLive2D;
class UUnLive2DMotion;
class UMaterialInterface;
class FUnLive2DRawModel;


UCLASS(ClassGroup = UnLive2D, meta = (BlueprintSpawnableComponent), hidecategories=(Material,Mesh))
class UNLIVE2DASSET_API UUnLive2DRendererComponent : public UProceduralMeshComponent
{
	GENERATED_UCLASS_BODY()

public:

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual void OnRegister() override;

	virtual void OnUnregister() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif


protected:
	// 更新
	virtual void UpdateRenderer();

	// 初始化
	virtual void InitUnLive2D();

protected:
	// UnLive2D资源
	UPROPERTY(Category = Live2D, EditAnywhere)
		UUnLive2D* SourceUnLive2D;


public:
	// Live2D颜色混合模式为CubismBlendMode_Normal使用的材质
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Rendering)
		UMaterialInterface* UnLive2DNormalMaterial;

	// Live2D颜色混合模式为CubismBlendMode_Additive使用的材质
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Rendering)
		UMaterialInterface* UnLive2DAdditiveMaterial;

	// Live2D颜色混合模式为CubismBlendMode_Multiplicative使用的材质
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Rendering)
		UMaterialInterface* UnLive2DMultiplyMaterial;

public:
	UFUNCTION(BlueprintCallable, Category = "Live2D")
		bool SetUnLive2D(UUnLive2D* NewUnLive2D);

	UFUNCTION(BlueprintCallable, Category = "Live2D")
		UUnLive2D* GetUnLive2D(){ return SourceUnLive2D; }

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

	// 更新其他渲染参数
	UFUNCTION()
		void UpDataUnLive2DProperty();

	UFUNCTION()
		void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld);

	/** 动画播放完成 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Anim")
		void OnMotionPlayeEnd();

private:

	// Live2D模型设置模块
	TSharedPtr<FUnLive2DRawModel> UnLive2DRawModel;

public:
	// Live2D渲染模块
	TSharedPtr<class FUnLive2DRenderState> UnLive2DRander;
};