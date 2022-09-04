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
#include "UnLive2DSetting.h"

#if !UE_VERSION_OLDER_THAN(5,0,0)
#include "Math/Vector4.h"
#endif

#if WITH_EDITOR
#include "Id/CubismIdManager.hpp"
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

	TArray<FVector2D> InterlottingLive2DUV2;

	UMaterialInstanceDynamic* InterlottingDynamicMat;

public:
	FUnLive2DMeshSectionData(TArray<FVector>& Live2DVertexs, TArray<int32>& Live2DIndices, TArray<FVector2D>& Live2DUVs, TArray<FColor>& Live2DColors, TArray<FVector2D>& Live2DUV1, TArray<FVector2D>& Live2DUV2, UMaterialInstanceDynamic* DynamicMat)
		: InterlottingLive2DVertexs(MoveTemp(Live2DVertexs))
		, InterlottingLive2DIndices(MoveTemp(Live2DIndices))
		, InterlottingLive2DUVs(MoveTemp(Live2DUVs))
		, InterlottingLive2DColors(MoveTemp(Live2DColors))
		, InterlottingLive2DUV1(MoveTemp(Live2DUV1))
		, InterlottingLive2DUV2(MoveTemp(Live2DUV2))
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
		UnLive2DRendererComponent->CreateMeshSection(i, UnLive2DMeshSectionData[i].InterlottingLive2DVertexs, UnLive2DMeshSectionData[i].InterlottingLive2DIndices, TArray<FVector>(),	UnLive2DMeshSectionData[i].InterlottingLive2DUVs, UnLive2DMeshSectionData[i].InterlottingLive2DUV1, UnLive2DMeshSectionData[i].InterlottingLive2DUV2, TArray<FVector2D>(), UnLive2DMeshSectionData[i].InterlottingLive2DColors, TArray<FProcMeshTangent>(), false);
	}

	UnLive2DMeshSectionData.Empty();
}

void DrawSepMask_Normal(UUnLive2DRendererComponent* UnLive2DRendererComponent, Csm::CubismModel* Live2DModel, const Csm::csmInt32 DrawableIndex, class CubismClippingContext* ClipContext)
{
	float DepthOffset = 0.f; //深度


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
			MeshSectionData->InterlottingLive2DColors.Add(FColor(ChanelFlag.X * 255, ChanelFlag.Y * 255, ChanelFlag.Z * 255, ChanelFlag.W * 255));

		}
		else
		{
			MeshSectionData->InterlottingLive2DColors.Add(FColor::White);
		}

		MeshSectionData->InterlottingLive2DUV2.Add(FVector2D(Opacity, Opacity));
		MeshSectionData->InterlottingLive2DVertexs.Add(FVector(Position.X * 100, DepthOffset, Position.Y * 100));
		MeshSectionData->InterlottingLive2DUVs.Add(FVector2D(UVArray[VertexIndex * 2], 1 - UVArray[VertexIndex * 2 + 1]));// UE UV坐标与Live2D的Y坐标是相反的
	}

	const csmInt32 VertexIndexCount = Live2DModel->GetDrawableVertexIndexCount(DrawableIndex); // 获得Drawable的顶点索引个数

	check(VertexIndexCount > 0 && "Bad Index Count");

	const csmUint16* IndicesArray = const_cast<csmUint16*>(Live2DModel->GetDrawableVertexIndices(DrawableIndex)); //顶点索引

	for (int32 VertexIndex = 0; VertexIndex < VertexIndexCount; ++VertexIndex)
	{
		MeshSectionData->InterlottingLive2DIndices.Add(InterlottingIndiceIndex + (int32)IndicesArray[VertexIndex]);
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

	const UUnLive2DSetting* Setting = GetDefault<UUnLive2DSetting>();
	UnLive2DNormalMaterial = Cast<UMaterialInterface>(Setting->DefaultUnLive2DNormalMaterial_Mesh.TryLoad());

	UnLive2DAdditiveMaterial = Cast<UMaterialInterface>(Setting->DefaultUnLive2DAdditiveMaterial_Mesh.TryLoad());

	UnLive2DMultiplyMaterial = Cast<UMaterialInterface>(Setting->DefaultUnLive2DMultiplyMaterial_Mesh.TryLoad());

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

	if (UnLive2DRawModel.IsValid())
	{
		UnLive2DRawModel->OnUpDate(DeltaTime * SourceUnLive2D->PlayRate);
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
					UnLive2DRander = MakeShared<FUnLive2DRenderState>(this, GetWorld());
					UnLive2DRander->SetUnLive2DMaterial(0, UnLive2DNormalMaterial);
					UnLive2DRander->SetUnLive2DMaterial(1, UnLive2DAdditiveMaterial);
					UnLive2DRander->SetUnLive2DMaterial(2, UnLive2DMultiplyMaterial);
				}
				UnLive2DRander->InitRender(SourceUnLive2D, UnLive2DRawModel);
			}
			MarkRenderStateDirty();
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif

void UUnLive2DRendererComponent::UpdateRenderer()
{
	if (!UnLive2DRawModel.IsValid()) return;

	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel->GetModel();

	if (UnLive2DModel == nullptr || !UnLive2DRander.IsValid()) return;

	// 限幅掩码・缓冲前处理方式的情况
	UnLive2DRander->UpdateRenderBuffers(UnLive2DRawModel);
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
	UWorld* World = GetWorld();

	if (World == nullptr || World->bIsTearingDown || SourceUnLive2D == nullptr ) return;


	if (!UnLive2DRawModel.IsValid())
	{
		UnLive2DRawModel = SourceUnLive2D->CreateLive2DRawModel();
		UnLive2DRawModel->OnMotionPlayEnd.BindUObject(this, &UUnLive2DRendererComponent::OnMotionPlayeEnd);
	}
	else
	{
		if (UnLive2DRawModel->GetOwnerLive2D().IsValid() && UnLive2DRawModel->GetOwnerLive2D().Get() != SourceUnLive2D)
		{
			UnLive2DRawModel.Reset();
			UnLive2DRawModel = SourceUnLive2D->CreateLive2DRawModel();
			UnLive2DRawModel->OnMotionPlayEnd.BindUObject(this, &UUnLive2DRendererComponent::OnMotionPlayeEnd);
		}
	}

	if (!UnLive2DRander.IsValid())
	{
		UnLive2DRander = MakeShared<FUnLive2DRenderState>(this, GetWorld());
		UnLive2DRander->SetUnLive2DMaterial(0, UnLive2DNormalMaterial);
		UnLive2DRander->SetUnLive2DMaterial(1, UnLive2DAdditiveMaterial);
		UnLive2DRander->SetUnLive2DMaterial(2, UnLive2DMultiplyMaterial);
	}
	
	UnLive2DRander->InitRender(SourceUnLive2D, UnLive2DRawModel);
}

bool UUnLive2DRendererComponent::SetUnLive2D(UUnLive2D* NewUnLive2D)
{
	if (NewUnLive2D == nullptr) return false;
	
	if(NewUnLive2D == SourceUnLive2D) return false;

	if ((GetOwner() != nullptr) && !AreDynamicDataChangesAllowed()) return false;

	SourceUnLive2D = NewUnLive2D;

	SourceUnLive2D->OnUpDataUnLive2DProperty.BindUObject(this, &UUnLive2DRendererComponent::UpDataUnLive2DProperty);

	InitUnLive2D();

	//SourceUnLive2D->SetOwnerObject(this);

	return true;
}

void UUnLive2DRendererComponent::PlayMotion(UUnLive2DMotion* InMotion)
{
	if (UnLive2DRawModel.IsValid())
	{
		UnLive2DRawModel->StartMotion(InMotion);
	}
}

void UUnLive2DRendererComponent::PlayExpression(UUnLive2DExpression* InExpression)
{
	if (UnLive2DRawModel.IsValid())
	{
		UnLive2DRawModel->StartExpressions(InExpression);
	}
}

void UUnLive2DRendererComponent::StopMotion()
{
	if (UnLive2DRawModel.IsValid())
	{
		UnLive2DRawModel->StopMotion();
	}
}

#if WITH_EDITOR
bool UUnLive2DRendererComponent::GetModelParamterGroup(TArray<FUnLive2DParameterData>& ParameterArr)
{
	if (!UnLive2DRawModel.IsValid()) return false;

	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel->GetModel();

	UnLive2DRawModel->SetBreathAnimAutoPlay(false);

	if (UnLive2DModel == nullptr) return false;

	ParameterArr.Empty();

	const Csm::csmInt32 ParameterCount = UnLive2DModel->GetParameterCount();

	const csmChar** ParameterIds = UnLive2DRawModel->GetLive2DModelParameterIds();

	for (Csm::csmInt32 i = 0; i < ParameterCount; i++)
	{
		const char* ParameterIDName = ParameterIds[i];

		ParameterArr.Add(FUnLive2DParameterData(
		i,
		ParameterIDName,
		UnLive2DModel->GetParameterValue(i), // 当前值
		UnLive2DModel->GetParameterDefaultValue(i), // 默认值
		UnLive2DModel->GetParameterMinimumValue(i), // 最小值
		UnLive2DModel->GetParameterMaximumValue(i))); // 最大值
	}

	return true;
}

void UUnLive2DRendererComponent::SetModelParamterValue(int32  ParameterID, float NewParameter, EUnLive2DExpressionBlendType::Type BlendType)
{
	if (!UnLive2DRawModel.IsValid()) return;

	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel->GetModel();

	if (UnLive2DModel == nullptr) return;

	switch (BlendType)
	{
	case EUnLive2DExpressionBlendType::ExpressionBlendType_Add:
		UnLive2DModel->AddParameterValue(ParameterID, NewParameter);
		break;
	case EUnLive2DExpressionBlendType::ExpressionBlendType_Multiply:
		UnLive2DModel->MultiplyParameterValue(ParameterID, NewParameter);
		break;
	case EUnLive2DExpressionBlendType::ExpressionBlendType_Overwrite:
		UnLive2DModel->SetParameterValue(ParameterID, NewParameter);
		break;
	}

	UnLive2DModel->SaveParameters();
}

bool UUnLive2DRendererComponent::GetModelParamterIDData(FName ParameterStr, FUnLive2DParameterData& Parameter)
{
	const CubismIdHandle ParameterId = CubismFramework::GetIdManager()->GetId(TCHAR_TO_UTF8(*ParameterStr.ToString())); // 参数标识
	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel->GetModel();
	if (UnLive2DModel == nullptr) return false;

	int32 ID;
	if (!GetModelParamterID(ParameterStr, ID))
	{
		return false;
	}
	Parameter = (FUnLive2DParameterData(
		ID,
		ParameterStr,
		UnLive2DModel->GetParameterValue(ID), // 当前值
		UnLive2DModel->GetParameterDefaultValue(ID), // 默认值
		UnLive2DModel->GetParameterMinimumValue(ID), // 最小值
		UnLive2DModel->GetParameterMaximumValue(ID))); // 最大值

	return true;
}

bool UUnLive2DRendererComponent::GetModelParamterID(FName ParameterStr, int32& ParameterID)
{
	const CubismIdHandle ParameterHandle = CubismFramework::GetIdManager()->GetId(TCHAR_TO_UTF8(*ParameterStr.ToString())); // 参数标识
	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel->GetModel();
	if (UnLive2DModel == nullptr) return false;


	ParameterID = UnLive2DModel->GetParameterIndex(ParameterHandle);
	return true;
}

#endif

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

void UUnLive2DRendererComponent::OnMotionPlayeEnd_Implementation()
{

}

