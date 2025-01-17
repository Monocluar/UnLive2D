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
#include "PhysicsEngine/BodySetup.h"

#if !UE_VERSION_OLDER_THAN(5,0,0)
#include "Math/Vector4.h"
#endif

#if WITH_EDITOR
#include "Id/CubismIdManager.hpp"
#endif
#include "Draw/UnLive2DSceneProxy.h"
#include "Draw/UnLive2DTargetBoxProxy.h"

using namespace Csm;

UUnLive2DRendererComponent::UUnLive2DRendererComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SourceUnLive2D(nullptr)
	, UnLive2DRenderType(EUnLive2DRenderType::RenderTarget)
	, UnLive2DSceneProxy(nullptr)
{
	BoundsScale = 1024.f;
	if (!ObjectInitializer.GetObj()->HasAnyFlags(RF_ClassDefaultObject))
	{
		PrimaryComponentTick.bCanEverTick = true;
		bTickInEditor = true;
		bAutoActivate = true;
	}

	const UUnLive2DSetting* Setting = GetDefault<UUnLive2DSetting>();
	UnLive2DNormalMaterial = Setting->DefaultUnLive2DNormalMaterial_Mesh;

	UnLive2DAdditiveMaterial = Setting->DefaultUnLive2DAdditiveMaterial_Mesh;

	UnLive2DMultiplyMaterial = Setting->DefaultUnLive2DMultiplyMaterial_Mesh;

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
		//UpdateRenderer();
	}
	if (UnLive2DSceneProxy)
	{
		UnLive2DSceneProxy->OnUpData();
	}

#endif
}

void UUnLive2DRendererComponent::OnRegister()
{
	Super::OnRegister();

	if (SourceUnLive2D == nullptr) return;

#if !UE_SERVER

	//FWorldDelegates::LevelRemovedFromWorld.AddUObject(this, &UUnLive2DRendererComponent::OnLevelRemovedFromWorld);

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

#if WITH_EDITOR
void UUnLive2DRendererComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FProperty* Property = PropertyChangedEvent.MemberProperty;

	if (Property && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		FName PropertyName = Property->GetFName();


		if (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(UUnLive2DRendererComponent, SourceUnLive2D))
		{
			MarkRenderStateDirty();
		}
		else if (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(UUnLive2DRendererComponent, UnLive2DRenderType))
		{
			MarkRenderStateDirty();
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif


FPrimitiveSceneProxy* UUnLive2DRendererComponent::CreateSceneProxy()
{
	UnLive2DSceneProxy = nullptr;
	if (UnLive2DRenderType == EUnLive2DRenderType::Mesh)
		UnLive2DSceneProxy = new FUnLive2DSceneProxy(this);
	else
		UnLive2DSceneProxy = new FUnLive2DTargetBoxProxy(this);
	return UnLive2DSceneProxy;
}

void UUnLive2DRendererComponent::PostLoad()
{
	Super::PostLoad();

	if (ProcMeshBodySetup && IsTemplate())
	{
		ProcMeshBodySetup->SetFlags(RF_Public | RF_ArchetypeObject);
	}
}

UBodySetup* UUnLive2DRendererComponent::GetBodySetup()
{
	if (ProcMeshBodySetup == nullptr)
	{
		UBodySetup* NewBodySetup = NewObject<UBodySetup>(this, NAME_None, (IsTemplate() ? RF_Public | RF_ArchetypeObject : RF_NoFlags));
		NewBodySetup->BodySetupGuid = FGuid::NewGuid();

		NewBodySetup->bGenerateMirroredCollision = false;
		NewBodySetup->bDoubleSidedGeometry = true;
		NewBodySetup->CollisionTraceFlag = CTF_UseComplexAsSimple ;
		ProcMeshBodySetup = NewBodySetup;
	}
	return ProcMeshBodySetup;
}


void UUnLive2DRendererComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials /*= false*/) const
{
	if (UnLive2DSceneProxy)
	{
		UnLive2DSceneProxy->GetUsedMaterials(OutMaterials, bGetDebugMaterials);
	}
}

FBoxSphereBounds UUnLive2DRendererComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	//return Super::CalcBounds(LocalToWorld);
	FBoxSphereBounds Ret(LocalBounds.TransformBy(LocalToWorld));

	Ret.BoxExtent *= BoundsScale;
	Ret.SphereRadius *= BoundsScale;

	return Ret;
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

	if (UnLive2DRawModel.IsValid())
	{
		if (Csm::CubismModel* UnLive2DModel = UnLive2DRawModel->GetModel())
		{
			csmFloat32 CanvasWidth = UnLive2DModel->GetCanvasWidth();
			csmFloat32 CanvasHeight = UnLive2DModel->GetCanvasHeight();
			FBox LocalBox(FVector(-CanvasWidth, 0, -CanvasHeight), FVector(CanvasWidth, 0, CanvasHeight));
			LocalBounds = FBoxSphereBounds(LocalBox);
		}
	}

	// Update global bounds
	UpdateBounds();
	// Need to send to render thread
	MarkRenderTransformDirty();
}

bool UUnLive2DRendererComponent::SetUnLive2D(UUnLive2D* NewUnLive2D)
{
	if (NewUnLive2D == nullptr) return false;
	
	if(NewUnLive2D == SourceUnLive2D) return false;

	if ((GetOwner() != nullptr) && !AreDynamicDataChangesAllowed()) return false;

	SourceUnLive2D = NewUnLive2D;

	//SourceUnLive2D->OnUpDataUnLive2DProperty.BindUObject(this, &UUnLive2DRendererComponent::UpDataUnLive2DProperty);

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


void UUnLive2DRendererComponent::OnMotionPlayeEnd_Implementation()
{

}

