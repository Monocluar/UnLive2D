#include "UnLive2DRendererComponent.h"
#include "FWPort/UnLive2DModelRender.h"
#include "FWPort/UnLive2DRawModel.h"
#include "UnLive2D.h"
#include "CubismConfig.h"
#include "Draw/UnLive2DSepRenderer.h"

#include "CubismFramework.hpp"
#include "Model/CubismModel.hpp"
#include "Type/CubismBasicType.hpp"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/EngineVersionComparison.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "Framework/Application/SlateApplication.h"

#if !UE_VERSION_OLDER_THAN(5,0,0)
#include "Math/Vector4.h"
#endif

using namespace Csm;

#if UE_VERSION_OLDER_THAN(5,0,0)
typedef FMatrix FUnLiveMatrix;
typedef FVector4 FUnLiveVector4;
#else
using namespace UE::Math;
typedef FMatrix44f FUnLiveMatrix;
typedef FVector4f FUnLiveVector4;
#endif

struct FUnLive2DMeshSectionData
{
public:
	TArray<FVector> InterlottingLive2DVertexs;

	TArray<int32> InterlottingLive2DIndices;

	TArray<FVector2D> InterlottingLive2DUVs;

	TArray<FColor> InterlottingLive2DColors;

	TArray<FVector2D> InterlottingLive2DUV1;

	UMaterialInstanceDynamic* InterlottingDynamicMat;

public:
	FUnLive2DMeshSectionData(TArray<FVector>& Live2DVertexs, TArray<int32>& Live2DIndices, TArray<FVector2D>& Live2DUVs, TArray<FColor>& Live2DColors, TArray<FVector2D>& Live2DUV1, UMaterialInstanceDynamic* DynamicMat)
		: InterlottingLive2DVertexs(MoveTemp(Live2DVertexs))
		, InterlottingLive2DIndices(MoveTemp(Live2DIndices))
		, InterlottingLive2DUVs(MoveTemp(Live2DUVs))
		, InterlottingLive2DColors(MoveTemp(Live2DColors))
		, InterlottingLive2DUV1(MoveTemp(Live2DUV1))
		, InterlottingDynamicMat(DynamicMat)
	{}

	FUnLive2DMeshSectionData()
		: InterlottingDynamicMat(nullptr)
	{}
};

TArray<FUnLive2DMeshSectionData> UnLive2DMeshSectionData;


void DrawMeshMeshSection(UUnLive2DRendererComponent* UnLive2DRendererComponent)
{
	for (int32 i = 0; i < UnLive2DMeshSectionData.Num(); i++)
	{
		UnLive2DRendererComponent->SetMaterial(i, UnLive2DMeshSectionData[i].InterlottingDynamicMat);
		UnLive2DRendererComponent->CreateMeshSection(i, UnLive2DMeshSectionData[i].InterlottingLive2DVertexs, UnLive2DMeshSectionData[i].InterlottingLive2DIndices, TArray<FVector>(),	UnLive2DMeshSectionData[i].InterlottingLive2DUVs, UnLive2DMeshSectionData[i].InterlottingLive2DUV1, TArray<FVector2D>(), TArray<FVector2D>(), UnLive2DMeshSectionData[i].InterlottingLive2DColors, TArray<FProcMeshTangent>(), false);
	}

	UnLive2DMeshSectionData.Empty();
}

void DrawSepMask_Normal(UUnLive2DRendererComponent* UnLive2DRendererComponent, Csm::CubismModel* Live2DModel, const Csm::csmInt32 DrawableIndex, class CubismClippingContext* ClipContext)
{
	float DepthOffset = 0 ; //深度

	csmFloat32 Opacity = Live2DModel->GetDrawableOpacity(DrawableIndex); // 获取不透明度
	if (Opacity <= 0.f) return;

	UMaterialInstanceDynamic* DynamicMat = UnLive2DRendererComponent->UnLive2DRander->GetMaterialInstanceDynamicToIndex(Live2DModel, DrawableIndex, ClipContext != nullptr); // 获取当前动态材质

	if (DynamicMat == nullptr) return;

	FUnLive2DMeshSectionData* MeshSectionData = nullptr;

	if (UnLive2DMeshSectionData.Num() == 0 || UnLive2DMeshSectionData.Top().InterlottingDynamicMat != DynamicMat)
	{
		MeshSectionData = &UnLive2DMeshSectionData.AddDefaulted_GetRef();
		MeshSectionData->InterlottingDynamicMat = DynamicMat;
	}
	else
	{
		MeshSectionData = &UnLive2DMeshSectionData.Top();
	}

	int32 InterlottingIndiceIndex = MeshSectionData->InterlottingLive2DVertexs.Num();

	const csmInt32 NumVertext = Live2DModel->GetDrawableVertexCount(DrawableIndex); // 获得Drawable顶点的个数

	const csmFloat32* UVArray = reinterpret_cast<csmFloat32*>(const_cast<Live2D::Cubism::Core::csmVector2*>(Live2DModel->GetDrawableVertexUvs(DrawableIndex))); // 获取UV组
	const csmFloat32* VertexArray = const_cast<csmFloat32*>(Live2DModel->GetDrawableVertices(DrawableIndex)); // 顶点组

	FUnLiveVector4 ChanelFlag;
	FUnLiveMatrix MartixForDraw = UnLive2DRendererComponent->UnLive2DRander->GetUnLive2DPosToClipMartix(ClipContext, ChanelFlag);


	for (int32 VertexIndex = 0; VertexIndex < NumVertext; ++VertexIndex)
	{

#if UE_VERSION_OLDER_THAN(5,0,0)
		FVector4 Position = FVector4(VertexArray[VertexIndex * 2], VertexArray[VertexIndex * 2 + 1], 0, 1);
#else
		TVector4<float> Position = TVector4<float>(VertexArray[VertexIndex * 2], VertexArray[VertexIndex * 2 + 1], 0, 1);
#endif

		float MaskVal = 1;
		if (ClipContext != nullptr)
		{
#if UE_VERSION_OLDER_THAN(5,0,0)
			FVector4 ClipPosition;
#else
			TVector4<float> ClipPosition;
#endif

			ClipPosition = MartixForDraw.TransformFVector4(Position);
			FVector2D MaskUV = FVector2D(ClipPosition.X, 1 + ClipPosition.Y);
			MaskUV /= ClipPosition.W;

			MeshSectionData->InterlottingLive2DUV1.Add(MaskUV);
			MeshSectionData->InterlottingLive2DColors.Add(FColor(ChanelFlag.X * 255, ChanelFlag.Y * 255, ChanelFlag.Z * 255, Opacity * 255));

		}
		else
		{
			MeshSectionData->InterlottingLive2DColors.Add(FColor(255, 255, 255, Opacity * 255));
		}


		MeshSectionData->InterlottingLive2DVertexs.Add(FVector(Position.X * 100, DepthOffset, Position.Y * 100));
		MeshSectionData->InterlottingLive2DUVs.Add(FVector2D(UVArray[VertexIndex * 2], 1 - UVArray[VertexIndex * 2 + 1]));// UE UV坐标与Live2D的Y坐标是相反的
	}

	const csmInt32 VertexIndexCount = Live2DModel->GetDrawableVertexIndexCount(DrawableIndex); // 获得Drawable的顶点索引个数

	check(VertexIndexCount > 0 && "Bad Index Count");

	const csmUint16* IndicesArray = const_cast<csmUint16*>(Live2DModel->GetDrawableVertexIndices(DrawableIndex)); //顶点索引

	for (int32 VertexIndex = 0; VertexIndex < VertexIndexCount; ++VertexIndex)
	{
		MeshSectionData->InterlottingLive2DIndices.AddDefaulted_GetRef() = InterlottingIndiceIndex + (int32)IndicesArray[VertexIndex];
	}

}

void DrawSepMask(UUnLive2DRendererComponent* UnLive2DRendererComponent, Csm::CubismModel* Live2DModel, const Csm::csmInt32 DrawableIndex, class CubismClippingContext* ClipContext)
{
	if (Live2DModel == nullptr) return;

	DrawSepMask_Normal(UnLive2DRendererComponent, Live2DModel, DrawableIndex, ClipContext);
}

void DrawSepNormal(UUnLive2DRendererComponent* UnLive2DRendererComponent, Csm::CubismModel* Live2DModel, const Csm::csmInt32 DrawableIndex)
{
	DrawSepMask_Normal(UnLive2DRendererComponent, Live2DModel, DrawableIndex, nullptr);
}


UUnLive2DRendererComponent::UUnLive2DRendererComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SourceUnLive2D(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	bAutoActivate = true;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> NormalMaterial(TEXT("/UnLive2DAsset/Mesh/UnLive2DPassNormalMaterial"));
	UnLive2DNormalMaterial = NormalMaterial.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> AdditiveMaterial(TEXT("/UnLive2DAsset/Mesh/UnLive2DPassAdditiveMaterial"));
	UnLive2DAdditiveMaterial = AdditiveMaterial.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MultiplyMaterial(TEXT("/UnLive2DAsset/Mesh/UnLive2DPassMultiplyMaterial"));
	UnLive2DMultiplyMaterial = MultiplyMaterial.Object;

	TextureParameterName = TEXT("UnLive2D");
}


void UUnLive2DRendererComponent::BeginPlay()
{
	Super::BeginPlay();

	InitUnLive2D();
}

void UUnLive2DRendererComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if !UE_SERVER
	//if (GetOwner() == nullptr) return;

	if (IsRunningDedicatedServer()) return;

	if (SourceUnLive2D == nullptr) return;

	TWeakPtr<FUnLive2DRawModel> WeakPtr = GetUnLive2D()->GetUnLive2DRawModel();
	if (WeakPtr.IsValid())
	{

		WeakPtr.Pin()->OnUpDate(DeltaTime * SourceUnLive2D->PlayRate);
		UpdateRenderer();
	}


#endif
}

void UUnLive2DRendererComponent::OnRegister()
{
	Super::OnRegister();

	if (SourceUnLive2D == nullptr) return;

#if !UE_SERVER

	FWorldDelegates::LevelRemovedFromWorld.AddUObject(this, &UUnLive2DRendererComponent::OnLevelRemovedFromWorld);

	if (IsRunningDedicatedServer()) return;

#if WITH_EDITOR

	const bool bIsGameWorld = GetWorld()->IsGameWorld();

	if (!bIsGameWorld)
	{
		InitUnLive2D();
	}
#endif

#endif
}

void UUnLive2DRendererComponent::OnUnregister()
{
	if (GetWorld() && UnLive2DRander.IsValid())
	{
		UnLive2DRander.Reset();
	}

	Super::OnUnregister();
}

#if WITH_EDITOR
void UUnLive2DRendererComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FProperty* Property = PropertyChangedEvent.MemberProperty;

	if (Property && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		FName PropertyName = Property->GetFName();


		if (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(UUnLive2DRendererComponent, SourceUnLive2D))
		{
			if (SourceUnLive2D )
			{
				if (!UnLive2DRander.IsValid())
				{
					UnLive2DRander = MakeShared<FUnLive2DRenderState>(this);
				}
				UnLive2DRander->InitRender(SourceUnLive2D);
			}
			MarkRenderStateDirty();
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif

void UUnLive2DRendererComponent::UpdateRenderer()
{
	if (!SourceUnLive2D->GetUnLive2DRawModel().IsValid()) return;

	Csm::CubismModel* UnLive2DModel = SourceUnLive2D->GetUnLive2DRawModel().Pin()->GetModel();

	if (UnLive2DModel == nullptr || !UnLive2DRander.IsValid()) return;

	// 限幅掩码・缓冲前处理方式的情况
	UnLive2DRander->UpdateRenderBuffers();
	/*if (UnLive2DModel->IsUsingMasking() && UnLive2DClippingManager.IsValid())
	{
		UnLive2DClippingManager->SetupClippingContext()
	}*/

	ClearAllMeshSections();

	const Csm::csmInt32 DrawableCount = UnLive2DModel->GetDrawableCount();
	const Csm::csmInt32* RenderOrder = UnLive2DModel->GetDrawableRenderOrders();

	TArray<Csm::csmInt32> SortedDrawableIndexList;
	SortedDrawableIndexList.SetNum(DrawableCount);

	for (csmInt32 i = 0; i < DrawableCount; i++)
	{
		const csmInt32 Order = RenderOrder[i];

		SortedDrawableIndexList[Order] = i;
	}

	// 描画
	for (csmInt32 i = 0; i < DrawableCount; i++)
	{
		const csmInt32 DrawableIndex = SortedDrawableIndexList[i];
		// <Drawable如果不是显示状态，则通过处理
		if (!UnLive2DModel->GetDrawableDynamicFlagIsVisible(DrawableIndex)) continue;

		if (0 == UnLive2DModel->GetDrawableVertexIndexCount(DrawableIndex)) continue;

		CubismClippingContext* ClipContext = UnLive2DRander->GetClipContextInDrawableIndex(DrawableIndex);

		const bool IsMaskDraw = (nullptr != ClipContext);

		if (IsMaskDraw)
		{
			DrawSepMask(this, UnLive2DModel, DrawableIndex, ClipContext);
		}
		else
		{
			DrawSepNormal(this, UnLive2DModel, DrawableIndex);
		}
	}

	DrawMeshMeshSection(this);
}

void UUnLive2DRendererComponent::InitUnLive2D()
{
	if (!FSlateApplication::IsInitialized()) return;

	UWorld* World = GetWorld();

	if (World == nullptr || World->bIsTearingDown || SourceUnLive2D == nullptr ) return;

	if (!UnLive2DRander.IsValid())
	{
		UnLive2DRander = MakeShared<FUnLive2DRenderState>(this);
		UnLive2DRander->InitRender(SourceUnLive2D);
	}
}

bool UUnLive2DRendererComponent::SetUnLive2D(UUnLive2D* NewUnLive2D)
{
	if(NewUnLive2D == SourceUnLive2D) return false;

	if ((GetOwner() != nullptr) && !AreDynamicDataChangesAllowed()) return false;

	SourceUnLive2D = NewUnLive2D;

	SourceUnLive2D->OnUpDataUnLive2DProperty.BindUObject(this, &UUnLive2DRendererComponent::UpDataUnLive2DProperty);

	if (UnLive2DRander.IsValid())
	{
		UnLive2DRander->InitRender(SourceUnLive2D);
	}
	else
	{
		InitUnLive2D();
	}

	SourceUnLive2D->SetOwnerObject(this);

	return true;
}

void UUnLive2DRendererComponent::UpDataUnLive2DProperty()
{
	if (!UnLive2DRander.IsValid()) return;

	UnLive2DRander->SetDynamicMaterialTintColor(SourceUnLive2D->TintColorAndOpacity);
}

void UUnLive2DRendererComponent::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	if (InLevel == nullptr || InWorld == GetWorld())
	{
		UnLive2DRander.Reset();
	}
}

