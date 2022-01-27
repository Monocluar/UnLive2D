
#pragma once

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "UnLive2DRendererComponent.generated.h"

class UUnLive2D;
class UMaterialInstanceDynamic;

UCLASS(ClassGroup = UnLive2D, meta = (BlueprintSpawnableComponent), hidecategories=(Material,Mesh))
class UNLIVE2DASSET_API UUnLive2DRendererComponent : public UProceduralMeshComponent
{
	GENERATED_UCLASS_BODY()

public:

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual void OnRegister() override;
	virtual void DestroyComponent(bool bPromoteChildren = false) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	// 更新
	virtual void UpdateRenderer();

	// 初始化
	virtual void InitUnLive2D();

public:
	// UnLive2D资源
	UPROPERTY(Category = Live2D, EditAnywhere)
		UUnLive2D* SourceUnLive2D;

	TSharedPtr<struct FUnLive2DRenderState> UnLive2DRander;

public:
	UFUNCTION(BlueprintCallable, Category = "Live2D")
		bool SetUnLive2D(UUnLive2D* NewUnLive2D);

protected:

	// 更新其他渲染参数
	UFUNCTION()
		void UpDataUnLive2DProperty();

};