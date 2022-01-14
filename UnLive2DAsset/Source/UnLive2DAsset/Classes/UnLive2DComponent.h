#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Components/MeshComponent.h"
#include "UnLive2DComponent.generated.h"

class UUnLive2D;
class UBodySetup;

UCLASS(Blueprintable, ClassGroup=UnLive2D, editinlinenew, meta=(BlueprintSpawnableComponent))
class UNLIVE2DASSET_API UUnLive2DComponent : public UMeshComponent
{
	GENERATED_UCLASS_BODY()


protected:
	UPROPERTY(Category = Live2D, EditAnywhere)
		UUnLive2D* SourceUnLive2D;

	// 匹配最理想的渲染大小
	UPROPERTY(EditAnywhere, Category = Live2D)
		bool bDrawAtDesiredSize;

	// 播放轴当前位置
	UPROPERTY()
		float AccumulatedTime;

	// 渲染目标附着到的材质的动态实例
	UPROPERTY(Transient, DuplicateTransient)
		UMaterialInstanceDynamic* MaterialInstance;

	// 将Live2D	呈现到的目标
	UPROPERTY(Transient, DuplicateTransient)
		UTextureRenderTarget2D* RenderTarget;

	// 显示的四元体的主体设置
	UPROPERTY(Transient, DuplicateTransient)
		class UBodySetup* BodySetup;

protected:
	// 实际绘图大小，这将根据DrawSize或小部件的所需大小（如果需要）进行更改
	UPROPERTY()
		FIntPoint CurrentDrawSize;

	/** 是否重新绘制 */
	bool bRedrawRequested;

	// UnLive2D 渲染模块
	TSharedPtr<struct FCubismSepRender> UnLive2DRender;

	// 我们最后一次渲染Live2D是什么时候？
	double LastWidgetRenderTime;


public:
	// 设置Live2D数据
	UFUNCTION(BlueprintCallable, Category = "Live2D")
		virtual bool SetUnLive2D(UUnLive2D* NewUnLive2D);

	// 获取Live2D数据
	UFUNCTION(BlueprintCallable, Category = "Live2D")
	FORCEINLINE UUnLive2D* GetUnLive2D(){ return SourceUnLive2D; };

	// 返回用于呈现 Live2D 的动态材质实例
	UFUNCTION(BlueprintCallable, Category = "Live2D")
		UMaterialInstanceDynamic* GetMaterialInstance() const;

	// 返回渲染目标
	UFUNCTION(BlueprintCallable, Category = "Live2D")
		UTextureRenderTarget2D* GetRenderTarget() const;

	// 设置此Live2D的着色颜色和不透明度比例
	UFUNCTION(BlueprintCallable, Category = "Live2D")
		void SetTintColorAndOpacity(const FLinearColor NewTintColorAndOpacity);

	// 设置渲染到视口时使用UI小部件纹理alpha的不透明度
	UFUNCTION(BlueprintCallable, Category = "Live2D")
		void SetOpacityFromTexture( const float NewOpacityFromTexture );

	// 更新UnLive2D渲染数据
	UFUNCTION()
		void UpDataUnLive2DRender();

	// 更新碰撞
	UFUNCTION()
		void UpDataUnLive2DBodySetup();

	// 更新其他渲染参数
	UFUNCTION()
		void UpDataUnLive2DProperty();

	// 返回用于此组件的线框颜色。
	FLinearColor GetWireframeColor() const;

	// 根据计时策略返回当前绝对时间
	double GetCurrentTime() const;

	// 计算矩形，找到仍然包含alpha高于AlphaThreshold的所有像素的最紧边界。
	bool FindTextureBoundingBox(/*out*/ FVector2D& OutBoxPosition, /*out*/ FVector2D& OutBoxSize);

protected:

	void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld);

	/** 
	* 确保主体设置已初始化，并在需要时进行更新。
	* @param bDrawSizeChanged 自上次更新调用以来，此组件的绘图大小是否已更改。
	*/
	void UpdateBodySetup( bool bDrawSizeChanged = false );

protected:

	// 初始化
	virtual void InitUnLive2D();

	// 释放与小部件关联的资源
	virtual void ReleaseResources();

	// 确保三维窗口是根据其大小和内容创建的
	virtual void UpdateUnLive2D();

	// 如果可能，将当前小部件绘制到渲染目标。
	virtual void DrawWidgetToRenderTarget(float DeltaTime);

	// 确保渲染目标已初始化，并在需要时对其进行更新。
	virtual void UpdateRenderTarget(FIntPoint DesiredRenderTargetSize);

	// 返回考虑GeometryMode的小部件组件的宽度
	float ComputeComponentWidth() const;

	// 更新材质实例
	void UpdateMaterialInstance();

	// 更新材质实例参数
	void UpdateMaterialInstanceParameters();

public:

	// UPrimitiveComponent interface
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	virtual UBodySetup* GetBodySetup() override;
	virtual FCollisionShape GetCollisionShape(float Inflation) const override;
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void DestroyComponent(bool bPromoteChildren = false) override;
	virtual UMaterialInterface* GetMaterial(int32 MaterialIndex) const override;
	virtual void SetMaterial(int32 ElementIndex, UMaterialInterface* Material) override;
	virtual int32 GetNumMaterials() const override;

	virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials = false) const override;
	// End of UPrimitiveComponent interface

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	FVector2D ConvertPivotSpaceToTextureSpace(FVector2D Input) const;
	FVector2D ConvertWorldSpaceToTextureSpace(const FVector& WorldPoint) const;

	FVector2D GetRawPivotPosition() const;

	FVector2D GetPivotPosition() const;

public:


	// UActorComponent interface
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual const UObject* AdditionalStatObject() const override;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// End of UActorComponent interface

};