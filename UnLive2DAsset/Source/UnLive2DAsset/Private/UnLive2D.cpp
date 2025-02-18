#include "UnLive2D.h"
#include "CubismBpLib.h"
#include "FWPort/UnLive2DRawModel.h"
#include "Engine/TextureRenderTarget2D.h"

#include "UnLive2DAssetModule.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "UnLive2D"


UUnLive2D::UUnLive2D(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Live2DScale(100)
	, TintColorAndOpacity(FLinearColor::White)
{
	//bTryLowPreciseMask = true;
	// 初始化Live2D库（只需要初始化一次）
	UCubismBpLib::InitCubism();
}

#if WITH_EDITOR
void UUnLive2D::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// Look for changed properties
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	const FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if ( MemberPropertyName == GET_MEMBER_NAME_STRING_CHECKED(UUnLive2D, TintColorAndOpacity))
	{
		if (OnUpDataUnLive2DProperty.IsBound()) OnUpDataUnLive2DProperty.Execute(TEXT("TintColorAndOpacity"));
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_STRING_CHECKED(UUnLive2D, Live2DScale))
	{
		if (OnUpDataUnLive2DProperty.IsBound()) OnUpDataUnLive2DProperty.Execute(TEXT("Live2DScale"));
	}
	 
}
#endif

TSharedPtr<FUnLive2DRawModel> UUnLive2D::CreateLive2DRawModel() const
{
	if (Live2DFileData.Live2DModelData.Num() == 0) return nullptr;

	TSharedPtr<FUnLive2DRawModel> UnLive2DRawModel = MakeShared<FUnLive2DRawModel>(this);
	const bool ReadSuc = UnLive2DRawModel->LoadAsset(Live2DFileData); 
	if (!ReadSuc)
	{
		UnLive2DRawModel.Reset();
	}

	return UnLive2DRawModel;
}

#if WITH_EDITOR
void UUnLive2D::LoadLive2DFileDataFormPath(const FString& InPath, TArray<FString>& TexturePaths, TArray<FUnLive2DMotionData>& LoadMotionData, TMap<FString, FUnLiveByteData>& LoadExpressionData)
{
	Live2DFileData = FUnLive2DRawModel::LoadLive2DFileDataFormPath(InPath, TexturePaths, LoadMotionData, LoadExpressionData);

}

#endif

const FUnLive2DLoadData* UUnLive2D::GetUnLive2DLoadData()
{
	return &Live2DFileData;
}

void UUnLive2D::PostLoad()
{
	Super::PostLoad();

}

#undef LOCTEXT_NAMESPACE