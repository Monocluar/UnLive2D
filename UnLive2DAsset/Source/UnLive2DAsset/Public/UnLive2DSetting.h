#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "UnLive2DSetting.generated.h"


UCLASS(config = Engine, defaultconfig)
class UNLIVE2DASSET_API UUnLive2DSetting : public UObject
{
	GENERATED_BODY()


// Mesh
public:

	// 默认的UnLive2DMesh_Normal材质
	UPROPERTY(config, EditAnywhere, meta = (AllowedClasses = "MaterialInterface", DisplayName= "DefaultNormalMaterial"), Category = "Mesh")
		FSoftObjectPath DefaultUnLive2DNormalMaterial_Mesh;

	// 默认的UnLive2DMesh_Additive材质
	UPROPERTY(config, EditAnywhere, meta = (AllowedClasses = "MaterialInterface", DisplayName = "DefaultAdditiveMaterial"), Category = "Mesh")
		FSoftObjectPath DefaultUnLive2DAdditiveMaterial_Mesh;

	// 默认的UnLive2DMesh_Multiply材质
	UPROPERTY(config, EditAnywhere, meta = (AllowedClasses = "MaterialInterface", DisplayName = "DefaultMultiplyMaterial"), Category = "Mesh")
		FSoftObjectPath DefaultUnLive2DMultiplyMaterial_Mesh;

// UI
public:
	// 默认的UnLive2DMesh_Normal材质
	UPROPERTY(config, EditAnywhere, meta = (AllowedClasses = "MaterialInterface", DisplayName = "DefaultNormalMaterial"), Category = "UI")
		FSoftObjectPath DefaultUnLive2DNormalMaterial_UI;

	// 默认的UnLive2DMesh_Additive材质
	UPROPERTY(config, EditAnywhere, meta = (AllowedClasses = "MaterialInterface", DisplayName = "DefaultAdditiveMaterial"), Category = "UI")
		FSoftObjectPath DefaultUnLive2DAdditiveMaterial_UI;

	// 默认的UnLive2DMesh_Multiply材质
	UPROPERTY(config, EditAnywhere, meta = (AllowedClasses = "MaterialInterface", DisplayName = "DefaultMultiplyMaterial"), Category = "UI")
		FSoftObjectPath DefaultUnLive2DMultiplyMaterial_UI;

// RenderTarget
public:
	UPROPERTY(config, EditAnywhere, meta = (AllowedClasses = "MaterialInterface", DisplayName = "RenderTargetMaterial"), Category = "RenderTarget")
	FSoftObjectPath DefaultUnLive2DRenderTargetMaterial;


public:
	UUnLive2DSetting();
};