#include "UnLive2DSetting.h"

UUnLive2DSetting::UUnLive2DSetting()
{
	DefaultUnLive2DNormalMaterial_Mesh = FSoftObjectPath("MaterialInstanceConstant'/UnLive2DAsset/Mesh/UnLive2DPassNormalMaterial.UnLive2DPassNormalMaterial'");
	DefaultUnLive2DAdditiveMaterial_Mesh = FSoftObjectPath("MaterialInstanceConstant'/UnLive2DAsset/Mesh/UnLive2DPassAdditiveMaterial.UnLive2DPassAdditiveMaterial'");
	DefaultUnLive2DMultiplyMaterial_Mesh = FSoftObjectPath("MaterialInstanceConstant'/UnLive2DAsset/Mesh/UnLive2DPassAdditiveMaterial.UnLive2DPassAdditiveMaterial'");

	DefaultUnLive2DNormalMaterial_UI = FSoftObjectPath("MaterialInstanceConstant'/UnLive2DAsset/Slate/UnLive2DSlateThrough_Normal.UnLive2DSlateThrough_Normal'");
	DefaultUnLive2DAdditiveMaterial_UI = FSoftObjectPath("MaterialInstanceConstant'/UnLive2DAsset/Slate/UnLive2DSlateThrough_Additive.UnLive2DSlateThrough_Additive'");
	DefaultUnLive2DMultiplyMaterial_UI = FSoftObjectPath("MaterialInstanceConstant'/UnLive2DAsset/Slate/UnLive2DSlateThrough_Multiply.UnLive2DSlateThrough_Multiply'");
}

