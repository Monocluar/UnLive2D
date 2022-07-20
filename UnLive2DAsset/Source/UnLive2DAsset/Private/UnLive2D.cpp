#include "UnLive2D.h"
#include "CubismBpLib.h"
#include "FWPort/UnLive2DRawModel.h"
#include "Engine/TextureRenderTarget2D.h"

#include "UnLive2DAssetModule.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "UnLive2D"


UUnLive2D::UUnLive2D(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, PlayRate(1.f)
	, TintColorAndOpacity(FLinearColor::White)
{

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
		if (OnUpDataUnLive2DProperty.IsBound())
		{
			OnUpDataUnLive2DProperty.Execute();
		}
	}

	 
}
#endif

void UUnLive2D::OnMotionPlayeEnd_Implementation()
{
}


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

void UUnLive2D::OnTap(const FVector2D& TapPosition)
{
	//if (!UnLive2DRawModel.IsValid()) return;

	/*FVector2D PointPos = TapPosition / DrawSize;

	UnLive2DRawModel->OnTapMotion(FVector2D(PointPos.X - 0.5f, 0.5f - PointPos.Y / * 因为UE4轴向和Live2D轴向不同，该Y轴向是相反的 * /) * 2);*/
}

void UUnLive2D::OnDrag(const FVector2D& DragPosition)
{
	//if (!UnLive2DRawModel.IsValid()) return;

	/*FVector2D PointPos = DragPosition / DrawSize;

	UnLive2DRawModel->SetDragPos(FVector2D(PointPos.X - 0.5f, 0.5f - PointPos.Y / * 因为UE4轴向和Live2D轴向不同，该Y轴向是相反的 * /) * 2);*/
}

void UUnLive2D::PostLoad()
{
	Super::PostLoad();

}

#undef LOCTEXT_NAMESPACE