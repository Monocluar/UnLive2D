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

	if ((PropertyChangedEvent.ChangeType & EPropertyChangeType::Interactive) == 0 )
	{
		InitLive2D();
	}
	 
}
#endif


void UUnLive2D::SetOwnerObject(UObject* Owner)
{
	OwnerObject = Owner;
}

void UUnLive2D::OnMotionPlayeEnd_Implementation()
{
}

void UUnLive2D::PlayMotion(UUnLive2DMotion* InMotion)
{
	if (UnLive2DRawModel.IsValid())
	{
		UnLive2DRawModel->StartMotion(InMotion);
	}
}

void UUnLive2D::PlayExpression(UUnLive2DExpression* InExpression)
{
	if (UnLive2DRawModel.IsValid())
	{
		UnLive2DRawModel->StartExpressions(InExpression);
	}
}

void UUnLive2D::InitLive2D()
{
	if (UnLive2DRawModel.IsValid()) return;

	/*if (SourceFilePath.FilePath.IsEmpty()) return;*/
	if (Live2DFileData.Live2DModelData.Num() == 0) return;

	UnLive2DRawModel = MakeShared<FUnLive2DRawModel>(this);

	const bool ReadSuc = UnLive2DRawModel->LoadAsset(Live2DFileData);
	if (!ReadSuc)
	{
		UnLive2DRawModel.Reset();
	}
	UnLive2DRawModel->OnMotionPlayEnd.BindUObject(this, &UUnLive2D::OnMotionPlayeEnd);

	if (OnUpDataUnLive2D.IsBound())
	{
		OnUpDataUnLive2D.Execute();
	}

}

#if WITH_EDITOR
void UUnLive2D::LoadLive2DFileDataFormPath(const FString& InPath, TArray<FString>& TexturePaths, TArray<FUnLive2DMotionData>& LoadMotionData, TMap<FString, FUnLiveByteData>& LoadExpressionData)
{
	Live2DFileData = FUnLive2DRawModel::LoadLive2DFileDataFormPath(InPath, TexturePaths, LoadMotionData, LoadExpressionData);

}

void UUnLive2D::GetModelParamterGroup()
{
	if (!UnLive2DRawModel.IsValid()) return;

	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel->GetModel();

	if (UnLive2DModel == nullptr) return;

	const Csm::csmInt32 ParameterCount = UnLive2DModel->GetParameterCount();

	const csmChar** ParameterIds = UnLive2DRawModel->GetLive2DModelParameterIds();

	TMap<FString, float> ParameterArr;

	for (Csm::csmInt32 i = 0; i < ParameterCount; i++)
	{
		Csm::csmFloat32 Parameter = UnLive2DModel->GetParameterValue(i);
		const char* ParameterIDName = ParameterIds[i];

		ParameterArr.Add(FString(ParameterIDName),Parameter);
	}

	ParameterArr.Num();
}

#endif

const FUnLive2DLoadData* UUnLive2D::GetUnLive2DLoadData()
{
	return &Live2DFileData;
}

void UUnLive2D::OnTap(const FVector2D& TapPosition)
{
	if (!UnLive2DRawModel.IsValid()) return;

	/*FVector2D PointPos = TapPosition / DrawSize;

	UnLive2DRawModel->OnTapMotion(FVector2D(PointPos.X - 0.5f, 0.5f - PointPos.Y / * 因为UE4轴向和Live2D轴向不同，该Y轴向是相反的 * /) * 2);*/
}

void UUnLive2D::OnDrag(const FVector2D& DragPosition)
{
	if (!UnLive2DRawModel.IsValid()) return;

	/*FVector2D PointPos = DragPosition / DrawSize;

	UnLive2DRawModel->SetDragPos(FVector2D(PointPos.X - 0.5f, 0.5f - PointPos.Y / * 因为UE4轴向和Live2D轴向不同，该Y轴向是相反的 * /) * 2);*/
}

void UUnLive2D::PostLoad()
{
	Super::PostLoad();

	InitLive2D();
}

#undef LOCTEXT_NAMESPACE