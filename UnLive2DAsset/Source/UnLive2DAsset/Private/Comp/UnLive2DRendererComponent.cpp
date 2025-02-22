#include "UnLive2DRendererComponent.h"
#include "FWPort/UnLive2DModelRender.h"
#include "FWPort/UnLive2DRawModel.h"
#include "UnLive2D.h"
#include "CubismConfig.h"

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

#if WITH_EDITOR
#include "Id/CubismIdManager.hpp"
#endif
#include "Draw/UnLive2DSceneProxy.h"
#include "Draw/UnLive2DTargetBoxProxy.h"
#include "Engine/TextureRenderTarget2D.h"


using namespace Csm;

UUnLive2DRendererComponent::UUnLive2DRendererComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SourceUnLive2D(nullptr)
	, PlayRate(1.f)
	, UnLive2DRenderType(EUnLive2DRenderType::Mesh)
	, RenderTargetSize(1024)
{
	//BoundsScale = 1024.f;
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

	UnLive2DRTMaterial = Setting->DefaultUnLive2DRenderTargetMaterial;
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
		UnLive2DRawModel->OnUpDate(DeltaTime * PlayRate);
		//UpdateRenderer();
	}
	if (UnLive2DProxyBase* UnLive2DSceneProxy = static_cast<UnLive2DProxyBase*>(SceneProxy))
	{
		if (UnLive2DSceneProxy->OnUpData())
		{
			LocalBounds = UnLive2DSceneProxy->GetLocalBox();
			MarkRenderDynamicDataDirty();
		}
	}

#endif
}

void UUnLive2DRendererComponent::SendRenderDynamicData_Concurrent()
{
	if (UnLive2DProxyBase* UnLive2DSceneProxy = static_cast<UnLive2DProxyBase*>(SceneProxy))
	{
		UnLive2DSceneProxy->OnUpDataRenderer();
	}
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
			InitUnLive2D();
			MarkRenderStateDirty();
		}
		else if (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(UUnLive2DRendererComponent, UnLive2DRenderType))
		{
			MarkRenderStateDirty();
		}

		static TArray<FName> MaterialPropertyArr = { GET_MEMBER_NAME_STRING_CHECKED(UUnLive2DRendererComponent, UnLive2DNormalMaterial),
			GET_MEMBER_NAME_STRING_CHECKED(UUnLive2DRendererComponent, UnLive2DAdditiveMaterial),
			GET_MEMBER_NAME_STRING_CHECKED(UUnLive2DRendererComponent, UnLive2DMultiplyMaterial),
			GET_MEMBER_NAME_STRING_CHECKED(UUnLive2DRendererComponent, UnLive2DRTMaterial) };

		if (MaterialPropertyArr.Contains(PropertyName))
		{
			MarkRenderStateDirty();
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif


FPrimitiveSceneProxy* UUnLive2DRendererComponent::CreateSceneProxy()
{
	if (SourceUnLive2D == nullptr) return nullptr;
	UnLive2DProxyBase* UnLive2DSceneProxy = nullptr;
	if (UnLive2DRenderType == EUnLive2DRenderType::Mesh)
		UnLive2DSceneProxy = new FUnLive2DSceneProxy(this);
	else
		UnLive2DSceneProxy = new FUnLive2DTargetBoxProxy(this);
	LocalBounds = UnLive2DSceneProxy->GetLocalBox();
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
	if (ProcMeshBodySetup == nullptr && UnLive2DRawModel.IsValid())
	{
		UBodySetup* NewBodySetup = NewObject<UBodySetup>(this);
		NewBodySetup->CollisionTraceFlag = CTF_UseSimpleAsComplex;
		NewBodySetup->AggGeom.BoxElems.Add(FKBoxElem());
		FKBoxElem* BoxElem = NewBodySetup->AggGeom.BoxElems.GetData();
		const float Width = LocalBounds.BoxExtent.X;
		const float Height = LocalBounds.BoxExtent.Y;
		const FVector Origin = FVector(.5f,
			-(Width)+(Width),
			-(Height)+(Height));

		BoxElem->X = 0.01f;
		BoxElem->Y = Width;
		BoxElem->Z = Height;

		BoxElem->SetTransform(FTransform::Identity);
		BoxElem->Center = Origin;
		ProcMeshBodySetup = NewBodySetup;

	}
	return ProcMeshBodySetup;
}


void UUnLive2DRendererComponent::BeginDestroy()
{
	Super::BeginDestroy();
	ClearRTCache();
}

FCollisionShape UUnLive2DRendererComponent::GetCollisionShape(float Inflation) const
{
	FVector BoxHalfExtent = (FVector(0.01f, LocalBounds.BoxExtent.X, LocalBounds.BoxExtent.Y) * GetComponentTransform().GetScale3D()) + Inflation;

	if (Inflation < 0.0f)
	{
		// Don't shrink below zero size.
		BoxHalfExtent = BoxHalfExtent.ComponentMax(FVector::ZeroVector);
	}

	return FCollisionShape::MakeBox(BoxHalfExtent);
}

void UUnLive2DRendererComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials /*= false*/) const
{
	if (UnLive2DProxyBase* UnLive2DSceneProxy = static_cast<UnLive2DProxyBase*>(SceneProxy))
	{
		UnLive2DSceneProxy->GetUsedMaterials(OutMaterials, bGetDebugMaterials);
	}
}

FBoxSphereBounds UUnLive2DRendererComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBoxSphereBounds Ret(LocalBounds.TransformBy(LocalToWorld));

	Ret.BoxExtent *= BoundsScale;
	Ret.SphereRadius *= BoundsScale;

	return Ret;
}

void UUnLive2DRendererComponent::InitUnLive2D()
{
	UWorld* World = GetWorld();

	if (World == nullptr || World->bIsTearingDown || SourceUnLive2D == nullptr) return;

#if WITH_EDITOR
	SourceUnLive2D->OnUpDataUnLive2DProperty.BindUObject(this, &UUnLive2DRendererComponent::UpDataUnLive2DProperty);
#endif

	UnLive2DRawModel.Reset();
	UnLive2DRawModel = SourceUnLive2D->CreateLive2DRawModel();
	UnLive2DRawModel->OnMotionPlayEnd.BindUObject(this, &UUnLive2DRendererComponent::OnMotionPlayeEnd);

	// Need to send to render thread
	MarkRenderStateDirty();

	// Update global bounds
	UpdateBounds();
}

bool UUnLive2DRendererComponent::SetUnLive2D(UUnLive2D* NewUnLive2D)
{
	if (NewUnLive2D == nullptr) return false;

	if (NewUnLive2D == SourceUnLive2D) return false;

	if ((GetOwner() != nullptr) && !AreDynamicDataChangesAllowed()) return false;

	SourceUnLive2D = NewUnLive2D;

	InitUnLive2D();

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

void UUnLive2DRendererComponent::UpDataUnLive2DProperty(FName PropertyName)
{
	if (PropertyName == TEXT("TintColorAndOpacity"))
	{
		if (UnLive2DProxyBase* UnLive2DSceneProxy = static_cast<UnLive2DProxyBase*>(SceneProxy))
		{
			UnLive2DSceneProxy->UpDataUnLive2DProperty(PropertyName);
		}
	}
	else if (PropertyName == TEXT("Live2DScale"))
	{
		// Need to send to render thread
		MarkRenderStateDirty();

		// Update global bounds
		UpdateBounds();
	}
}
#endif


void UUnLive2DRendererComponent::ClearRTCache()
{
	if (MaskBufferRenderTarget.IsValid())
	{
		MaskBufferRenderTarget->RemoveFromRoot();
		MaskBufferRenderTarget.Reset();
	}
	else
	{
		MaskBuffer.SafeRelease();
	}
}

void UUnLive2DRendererComponent::InitLive2DRenderData(EUnLive2DRenderType InRenderType,int32 BufferHeight)
{
	if (UnLive2DRenderType == EUnLive2DRenderType::Mesh)
	{
		if (MaskBufferRenderTarget.IsValid())
		{
			if (MaskBufferRenderTarget->SizeX == BufferHeight) return;
			MaskBufferRenderTarget->RemoveFromRoot();
			MaskBufferRenderTarget.Reset();
		}
		MaskBufferRenderTarget = NewObject<UTextureRenderTarget2D>(GetTransientPackage());
		MaskBufferRenderTarget->ClearColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
		MaskBufferRenderTarget->InitCustomFormat(BufferHeight, BufferHeight, EPixelFormat::PF_B8G8R8A8, false);
		MaskBufferRenderTarget->AddToRoot();
	}
	else
	{
		if (MaskBuffer.IsValid())
		{
			if (MaskBuffer->GetSizeXYZ().X == BufferHeight) return;
			MaskBuffer.SafeRelease();
		}

		ETextureCreateFlags Flags = ETextureCreateFlags(TexCreate_None | TexCreate_RenderTargetable | TexCreate_ShaderResource);
#if ENGINE_MAJOR_VERSION >= 5
		const FRHITextureCreateDesc Desc = FRHITextureCreateDesc::Create2D(TEXT("FUnLive2DTargetBoxProxy_UpdateSection_RenderThread"), BufferHeight, BufferHeight, PF_R8G8B8A8)
			.SetFlags(Flags).SetClearValue(FClearValueBinding(FLinearColor::White));

		MaskBuffer = RHICreateTexture(Desc);
#else
		FRHIResourceCreateInfo CreateInfo(TEXT("FUnLive2DTargetBoxProxy_UpdateSection_RenderThread"));
		CreateInfo.ClearValueBinding = FClearValueBinding(FLinearColor::White);
		MaskBuffer = RHICreateTexture2D(BufferHeight, BufferHeight, PF_R8G8B8A8, 1, 1, Flags, CreateInfo);
#endif
	}
}

FTextureRHIRef UUnLive2DRendererComponent::GetMaskTextureRHIRef() const
{
	if (UnLive2DRenderType == EUnLive2DRenderType::Mesh)
	{
		if (!MaskBufferRenderTarget.IsValid()) return FTextureRHIRef();
#if ENGINE_MAJOR_VERSION >=5 && ENGINE_MINOR_VERSION > 1
		const FTextureRHIRef RenderTargetTexture = MaskBufferRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
#else
		FRHITexture2D* RenderTargetTexture = MaskBufferRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
#endif
		return RenderTargetTexture;
	}
	else
		return MaskBuffer;
}


UTextureRenderTarget2D* UUnLive2DRendererComponent::GetTextureRenderTarget2D() const
{
	return MaskBufferRenderTarget.Get();
}

void UUnLive2DRendererComponent::OnMotionPlayeEnd_Implementation()
{

}

