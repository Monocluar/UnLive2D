#include "UnLive2D.h"
#include "CubismBpLib.h"
#include "FWPort/UnLive2DRawModel.h"
#include "Engine/TextureRenderTarget2D.h"

#include "UnLive2DAsset.h"
#include "Materials/MaterialInterface.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "UnLive2D"


UUnLive2D::UUnLive2D(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, PlayRate(1.f)
	, TintColorAndOpacity(FLinearColor::White)
{

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> NormalMaterial(TEXT("/UnLive2DAsset/UnLive2DPassNormalMaterial"));
	UnLive2DNormalMaterial = NormalMaterial.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> AdditiveMaterial(TEXT("/UnLive2DAsset/UnLive2DPassAdditiveMaterial"));
	UnLive2DAdditiveMaterial = AdditiveMaterial.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MultiplyMaterial(TEXT("/UnLive2DAsset/UnLive2DPassMultiplyMaterial"));
	UnLive2DMultiplyMaterial = MultiplyMaterial.Object;

	TextureParameterName = TEXT("UnLive2D");

	UnLive2DCollisionDomain = EUnLive2DCollisionMode::Use3DPhysics;

	// 初始化Live2D库（只需要初始化一次）
	UCubismBpLib::InitCubism();
}

#if WITH_EDITOR
void UUnLive2D::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// Look for changed properties
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	const FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;


	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UUnLive2D, PlayMotionIndex))
	{
		if (Live2DMotionGroup.IsValidIndex(PlayMotionIndex))
		{
			UnLive2DRawModel->PlayMotion(Live2DMotionGroup[PlayMotionIndex]);
		}

	}
	else if ( MemberPropertyName == GET_MEMBER_NAME_STRING_CHECKED(UUnLive2D, TintColorAndOpacity))
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


void UUnLive2D::CreatePolygonFromBoundingBox(FUnLive2DGeometryCollection& GeomOwner, bool bUseTightBounds)
{
	FVector2D BoxSize;
	FVector2D BoxPosition;

	if (bUseTightBounds)
	{
		FindTextureBoundingBox(BoxSize, BoxPosition);
	}
	else
	{
		BoxPosition = FVector2D::ZeroVector;
	}

	// 重新将箱子放在中间
	BoxPosition += BoxSize * 0.5f;

	const FVector2D HalfSize = BoxSize * 0.5f;

	new (GeomOwner.Vertices) FVector2D(-HalfSize.X, -HalfSize.Y);
	new (GeomOwner.Vertices) FVector2D(+HalfSize.X, -HalfSize.Y);
	new (GeomOwner.Vertices) FVector2D(+HalfSize.X, +HalfSize.Y);
	new (GeomOwner.Vertices) FVector2D(-HalfSize.X, +HalfSize.Y);

	GeomOwner.BoxSize = BoxSize;
	GeomOwner.BoxPosition = BoxPosition;
}

bool UUnLive2D::FindTextureBoundingBox(/*out*/ FVector2D& OutBoxPosition, /*out*/ FVector2D& OutBoxSize)
{
	/*if (OwnerObject.IsValid())
	{
		if (UUnLive2DComponent* Live2DComp = Cast<UUnLive2DComponent>(OwnerObject.Get()))
		{
			return Live2DComp->FindTextureBoundingBox(OutBoxPosition, OutBoxSize);
		}
	}*/

	return false;
}

void UUnLive2D::SetOwnerObject(UObject* Owner)
{
	OwnerObject = Owner;
}

void UUnLive2D::OnMotionPlayeEnd_Implementation()
{
}

void UUnLive2D::InitLive2D()
{
	if (UnLive2DRawModel.IsValid()) return;

	/*if (SourceFilePath.FilePath.IsEmpty()) return;*/
	if (Live2DFileData.Live2DModelData.Num() == 0) return;

	UnLive2DRawModel = MakeShared<FUnLive2DRawModel>();

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

	Live2DMotionGroup.Empty();
	for (typename TMap<FName, TArray<FName>>::TConstIterator Iterator(UnLive2DRawModel->GetAllMotionGroup()); Iterator; ++Iterator)
	{
		Live2DMotionGroup.Append(Iterator.Value());
;	}

}

#if WITH_EDITOR
void UUnLive2D::LoadLive2DFileDataFormPath(const FString& InPath)
{
	Live2DFileData = FUnLive2DRawModel::LoadLive2DFileDataFormPath(InPath);
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