#include "UnLive2DComponent.h"
#include "Net/UnrealNetwork.h"
#include "Draw/CubismSepRender.h"
#include "UnLive2DAsset.h"
#include "UnLive2DSceneProxy.h"
#include "UnLive2D.h"
#include "Misc/UObjectToken.h"
#include "Misc/MapErrors.h"
#include "Engine/TextureRenderTarget2D.h"
#include "FWPort/UnLive2DRawModel.h"

//DECLARE_CYCLE_STAT(TEXT("Tick UnLive2D"), STAT_TickUnLive2D, STATGROUP_UnLive2D);

#define LOCTEXT_NAMESPACE "UnLive2D"

FVector UnLive2DAxisX(0.0f, -1.0f, 0.0f);
FVector UnLive2DAxisY(0.0f, 0.0f, 1.0f);
FVector UnLive2DAxisZ(1.0f, 0.0f, 0.0f);

UUnLive2DComponent::UUnLive2DComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, LastWidgetRenderTime(0)
{
	SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);

	SetRelativeRotation(FRotator::ZeroRotator);

	CastShadow = false;
	bUseAsOccluder = false;
	bCanEverAffectNavigation = false;

	Mobility = EComponentMobility::Movable;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	bTickInEditor = true;

	AccumulatedTime = 0.f;

	bUseEditorCompositing = false;

	MaterialInstance = nullptr;

}

bool UUnLive2DComponent::SetUnLive2D(UUnLive2D* NewUnLive2D)
{
	if(NewUnLive2D == SourceUnLive2D) return false;

	if ((GetOwner() != nullptr) && !AreDynamicDataChangesAllowed()) return false;

	SourceUnLive2D = NewUnLive2D;

	SourceUnLive2D->OnUpDataUnLive2D.BindUObject(this, &UUnLive2DComponent::UpDataUnLive2DRender);
	SourceUnLive2D->OnUpDataUnLive2DBodySetup.BindUObject(this, &UUnLive2DComponent::UpDataUnLive2DBodySetup);
	SourceUnLive2D->OnUpDataUnLive2DProperty.BindUObject(this, &UUnLive2DComponent::UpDataUnLive2DProperty);

	UpDataUnLive2DRender();

	// 重置时间轴
	AccumulatedTime = 0.f;

	// 需要在某个点将其发送到渲染线程
	MarkRenderStateDirty();

	// 更新流数据
	IStreamingManager::Get().NotifyPrimitiveUpdated(this);

	// 更新边界
	UpdateBounds();

	SourceUnLive2D->SetOwnerObject(this);

	return true;
}

UMaterialInstanceDynamic* UUnLive2DComponent::GetMaterialInstance() const
{
	return MaterialInstance;
}

UTextureRenderTarget2D* UUnLive2DComponent::GetRenderTarget() const
{
	return RenderTarget;
}

void UUnLive2DComponent::SetTintColorAndOpacity(const FLinearColor NewTintColorAndOpacity)
{
	if (SourceUnLive2D == nullptr)
	{
		return;
	}
	if (NewTintColorAndOpacity != SourceUnLive2D->TintColorAndOpacity)
	{
		SourceUnLive2D->TintColorAndOpacity = NewTintColorAndOpacity;
		UpdateMaterialInstanceParameters();
	}
}

void UUnLive2DComponent::SetOpacityFromTexture(const float NewOpacityFromTexture)
{
	if (SourceUnLive2D == nullptr)
	{
		return;
	}
	if (NewOpacityFromTexture != SourceUnLive2D->OpacityFromTexture)
	{
		SourceUnLive2D->OpacityFromTexture = NewOpacityFromTexture;
		UpdateMaterialInstanceParameters();
	}
}

void UUnLive2DComponent::UpDataUnLive2DRender()
{
	if (UnLive2DRender.IsValid())
	{
		UnLive2DRender->InitRender(SourceUnLive2D);
	}
}

void UUnLive2DComponent::UpDataUnLive2DBodySetup()
{
	MarkRenderStateDirty();
	UpdateBodySetup(true);
	RecreatePhysicsState();
}

void UUnLive2DComponent::UpDataUnLive2DProperty()
{
	MarkRenderStateDirty();
	UpdateMaterialInstanceParameters();
}

FLinearColor UUnLive2DComponent::GetWireframeColor() const
{
	if (Mobility == EComponentMobility::Static)
	{
		return FColor(0, 255, 255, 255);
	}
	else
	{
		if (BodyInstance.bSimulatePhysics)
		{
			return FColor(0, 255, 128, 255);
		}
		else
		{
			return FColor(255, 0, 255, 255);
		}
	}
}

double UUnLive2DComponent::GetCurrentTime() const
{
	return FApp::GetCurrentTime();
}

bool UUnLive2DComponent::FindTextureBoundingBox(/*out*/ FVector2D& OutBoxPosition, /*out*/ FVector2D& OutBoxSize)
{
	if (SourceUnLive2D == nullptr) return false;


	if (RenderTarget == nullptr) return false;

	if (FTextureRenderTargetResource* RtResource = RenderTarget->GameThread_GetRenderTargetResource())
	{
		int32 LeftBound = 0;
		int32 RightBound = RtResource->GetSizeX() - 1;

		int32 TopBound = 0;
		int32 BottomBound = RtResource->GetSizeY() - 1;

		FColor* OutImageBytes = nullptr;
		RtResource->ReadPixelsPtr(OutImageBytes, RCM_UNorm);

		const FColor EmptyColor(ForceInitToZero);

		if (OutImageBytes == nullptr)
		{
			return false;
		}

		// Top
		{
			int32 Left = 0;
			while ((*(OutImageBytes + (TopBound * RtResource->GetSizeX()) + Left)).A == 255)
			{
				if (Left >= RightBound)
				{
					TopBound++;
					Left = 0;
				}
				else
				{
					Left++;
				}
			}
		}
		
		// Bottom
		{
			int32 Right = BottomBound;
			while ((*(OutImageBytes + (BottomBound * RtResource->GetSizeX()) + Right)).A == 255 && BottomBound > TopBound)
			{
				if (Right <= LeftBound)
				{
					BottomBound--;
					Right = RtResource->GetSizeX() - 1;
				}
				else
				{
					Right--;
				}
			}
		}
		
		// Left
		{
			int32 Top = TopBound;
			LeftBound = 0;
			while ((*(OutImageBytes + (Top * RtResource->GetSizeX()) + LeftBound)).A == 255)
			{
				if (Top >= BottomBound)
				{
					LeftBound++;
					Top = TopBound;
				}
				else
				{
					Top++;
				}
			}
		}
		
		// Right
		{
			int32 Bottom = BottomBound;

			while ((*(OutImageBytes + (Bottom * RtResource->GetSizeX()) + RightBound)).A == 255)
			{
				if (Bottom <= TopBound)
				{
					RightBound--;
					Bottom = BottomBound;
				}
				else
				{
					Bottom--;
				}
			}
		}
		

		OutBoxPosition.X = LeftBound;
		OutBoxPosition.Y = TopBound;
		OutBoxSize.X = RightBound - LeftBound + 1;
		OutBoxSize.Y = BottomBound - TopBound + 1;

		return true;
	}

	return false;
}

void UUnLive2DComponent::UpdateBodySetup(bool bDrawSizeChanged /*= false */)
{
	if (SourceUnLive2D == nullptr)
	{
		return;
	}

	if (!BodySetup || bDrawSizeChanged)
	{
		BodySetup = NewObject<UBodySetup>(this);
		BodySetup->CollisionTraceFlag = CTF_UseSimpleAsComplex;
		BodySetup->AggGeom.BoxElems.Add(FKBoxElem());

		FKBoxElem* BoxElem = BodySetup->AggGeom.BoxElems.GetData();

		const float Width = ComputeComponentWidth();
		const float Height = CurrentDrawSize.Y;
		const FVector Origin = FVector(.5f,
			-(Width * 0.5f) + (Width * SourceUnLive2D->Pivot.X),
			-(Height * 0.5f) + (Height * SourceUnLive2D->Pivot.Y));

		BoxElem->X = 0.01f;
		BoxElem->Y = Width;
		BoxElem->Z = Height;

		BoxElem->SetTransform(FTransform::Identity);
		BoxElem->Center = Origin;
	}
}

void UUnLive2DComponent::InitUnLive2D()
{
	if (!FSlateApplication::IsInitialized()) return;

	UWorld* World = GetWorld();

	if (!UnLive2DRender.IsValid() && World && !World->bIsTearingDown && SourceUnLive2D)
	{
		UnLive2DRender = MakeShared<FCubismSepRender>();
		UnLive2DRender->InitRender(SourceUnLive2D);
		SetComponentTickEnabled(true);
	}

	UpdateMaterialInstance();
}

void UUnLive2DComponent::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	if (InLevel == nullptr && InWorld == GetWorld())
	{
		ReleaseResources();
	}
}

void UUnLive2DComponent::ReleaseResources()
{
	if (UnLive2DRender.IsValid())
	{
		UnLive2DRender.Reset();
	}
}

void UUnLive2DComponent::UpdateUnLive2D()
{
	if (FSlateApplication::IsInitialized() && !IsPendingKill() && SourceUnLive2D)
	{
		bool bNeededNewWindow = false;
		
	}
}

void UUnLive2DComponent::DrawWidgetToRenderTarget(float DeltaTime)
{
	if (GUsingNullRHI) return;

	if (SourceUnLive2D == nullptr) return;

	const int32 MaxAllowedDrawSize = GetMax2DTextureDimension();
	if (SourceUnLive2D->DrawSize.X <= 0 || SourceUnLive2D->DrawSize.Y <= 0 || SourceUnLive2D->DrawSize.X > MaxAllowedDrawSize || SourceUnLive2D->DrawSize.Y > MaxAllowedDrawSize)
	{
		return;
	}

	const FIntPoint PreviousDrawSize = CurrentDrawSize;
	CurrentDrawSize = SourceUnLive2D->DrawSize;

	const float DrawScale = 1.0f;

	if (bDrawAtDesiredSize)
	{
		// TODO
	}

	if (CurrentDrawSize != PreviousDrawSize)
	{
		UpdateBodySetup(true);
		RecreatePhysicsState();
	}

	UpdateRenderTarget(CurrentDrawSize);

	if (! RenderTarget) return;

	bRedrawRequested = false;

	if (UnLive2DRender.IsValid() && SourceUnLive2D->GetUnLive2DRawModel().IsValid())
	{
		UnLive2DRender->DrawSeparateToRenderTarget(GetWorld(), RenderTarget);

		LastWidgetRenderTime = GetCurrentTime();
	}

}

void UUnLive2DComponent::UpdateRenderTarget(FIntPoint DesiredRenderTargetSize)
{
	if (!SourceUnLive2D)
	{
		return;
	}

	bool bWidgetRenderStateDirty = false;
	bool bClearColorChanged = false;

	FLinearColor ActualBackgroundColor = SourceUnLive2D->BackgroundColor;

	ActualBackgroundColor.A = 0.0f;

	if (DesiredRenderTargetSize.X != 0 && DesiredRenderTargetSize.Y != 0)
	{
		const EPixelFormat RequestedFormat = FSlateApplication::Get().GetRenderer()->GetSlateRecommendedColorFormat();

		if (RenderTarget == nullptr)
		{
			RenderTarget = NewObject<UTextureRenderTarget2D>(this);
			RenderTarget->ClearColor = ActualBackgroundColor;

			bClearColorChanged = bWidgetRenderStateDirty = true;

			RenderTarget->InitCustomFormat(DesiredRenderTargetSize.X, DesiredRenderTargetSize.Y, RequestedFormat, false);

			if (MaterialInstance)
			{
				MaterialInstance->SetTextureParameterValue("UnLive2D", RenderTarget);
			}
		}
		else
		{
			bClearColorChanged = (RenderTarget->ClearColor != ActualBackgroundColor);

			// 更新清晰的颜色或格式
			if (bClearColorChanged || RenderTarget->SizeX != DesiredRenderTargetSize.X || RenderTarget->SizeY != DesiredRenderTargetSize.Y)
			{
				RenderTarget->ClearColor = ActualBackgroundColor;
				RenderTarget->InitCustomFormat(DesiredRenderTargetSize.X, DesiredRenderTargetSize.Y, PF_B8G8R8A8, false);
				RenderTarget->UpdateResourceImmediate();
				bWidgetRenderStateDirty = true;
			}
		}
	}

	if (RenderTarget)
	{
		// 如果渲染目标的清晰颜色已更改，请更新材质的背景色以匹配
		if (bClearColorChanged && MaterialInstance)
		{
			MaterialInstance->SetVectorParameterValue("BackColor", RenderTarget->ClearColor);
		}

		if (bWidgetRenderStateDirty)
		{
			MarkRenderStateDirty();
		}
	}
}

float UUnLive2DComponent::ComputeComponentWidth() const
{
	return CurrentDrawSize.X;
}

void UUnLive2DComponent::UpdateMaterialInstance()
{
	if (MaterialInstance == nullptr)
	{
		UMaterialInterface* BaseMaterial = GetMaterial(0);
		if (!BaseMaterial) return;
		
		MaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	}

	UpdateMaterialInstanceParameters();

	MarkRenderStateDirty();
}

void UUnLive2DComponent::UpdateMaterialInstanceParameters()
{
	if (MaterialInstance == nullptr) return;

	if (SourceUnLive2D == nullptr) return;

	MaterialInstance->SetTextureParameterValue("UnLive2D", RenderTarget);
	MaterialInstance->SetVectorParameterValue("TintColorAndOpacity", SourceUnLive2D->TintColorAndOpacity);
	MaterialInstance->SetVectorParameterValue("BackColor", SourceUnLive2D->BackgroundColor);
	MaterialInstance->SetScalarParameterValue("OpacityFromTexture", SourceUnLive2D->OpacityFromTexture);
}

FPrimitiveSceneProxy* UUnLive2DComponent::CreateSceneProxy()
{
	if (SourceUnLive2D == nullptr || !UnLive2DRender.IsValid())
	{
		return UMeshComponent::CreateSceneProxy();
	}

	FUnLive2DSceneProxy* NewProxy = new FUnLive2DSceneProxy(this);

	LastWidgetRenderTime = 0;

	return NewProxy;
}

FBoxSphereBounds UUnLive2DComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	if (SourceUnLive2D == nullptr)
	{
		return Super::CalcBounds(LocalToWorld);
	}

	const FVector Origin = FVector(.5f,
		-(CurrentDrawSize.X * 0.5f) + (CurrentDrawSize.X * SourceUnLive2D->Pivot.X),
		-(CurrentDrawSize.Y * 0.5f) + (CurrentDrawSize.Y * SourceUnLive2D->Pivot.Y));

	const FVector BoxExtent = FVector(1.f, CurrentDrawSize.X / 2.0f, CurrentDrawSize.Y / 2.0f);

	FBoxSphereBounds NewBounds(Origin, BoxExtent, CurrentDrawSize.Size() / 2.0f);
	NewBounds = NewBounds.TransformBy(LocalToWorld);

	NewBounds.BoxExtent *= BoundsScale;
	NewBounds.SphereRadius *= BoundsScale;

	return NewBounds;
}

UMaterialInterface* UUnLive2DComponent::GetMaterial(int32 MaterialIndex) const
{
	if (OverrideMaterials.IsValidIndex(MaterialIndex) && (OverrideMaterials[MaterialIndex] != nullptr))
	{
		return OverrideMaterials[MaterialIndex];
	}


	return SourceUnLive2D == nullptr ? nullptr : SourceUnLive2D->UnLive2DMaterial;
}

void UUnLive2DComponent::SetMaterial(int32 ElementIndex, UMaterialInterface* Material)
{
	Super::SetMaterial(ElementIndex, Material);

	UpdateMaterialInstance();
}

int32 UUnLive2DComponent::GetNumMaterials() const
{
	return FMath::Max<int32>(OverrideMaterials.Num(), 1);
}

void UUnLive2DComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials /*= false*/) const
{
	if (MaterialInstance)
	{
		OutMaterials.AddUnique(MaterialInstance);
	}
}

#if WITH_EDITOR

void UUnLive2DComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FProperty* Property = PropertyChangedEvent.MemberProperty;

	if (Property && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		FName PropertyName = Property->GetFName();


		if (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(UUnLive2DComponent, SourceUnLive2D))
		{
			if (SourceUnLive2D && UnLive2DRender.IsValid())
			{
				UnLive2DRender->InitRender(SourceUnLive2D);
			}
			UpdateUnLive2D();
			MarkRenderStateDirty();
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif

UBodySetup* UUnLive2DComponent::GetBodySetup()
{
	UpdateBodySetup();
	return BodySetup;
}

FCollisionShape UUnLive2DComponent::GetCollisionShape(float Inflation) const
{
	FVector BoxHalfExtent = (FVector(0.01f, CurrentDrawSize.X * 0.5f, CurrentDrawSize.Y * 0.5f) * GetComponentTransform().GetScale3D()) + Inflation;

	if (Inflation < 0.0f)
	{
		// Don't shrink below zero size.
		BoxHalfExtent = BoxHalfExtent.ComponentMax(FVector::ZeroVector);
	}

	return FCollisionShape::MakeBox(BoxHalfExtent);
}

void UUnLive2DComponent::OnRegister()
{
	Super::OnRegister();
	// 在注册场景组件之前设置此选项，以便正确计算边界。
	if (SourceUnLive2D == nullptr)
	{
		return;
	}

	CurrentDrawSize = SourceUnLive2D->DrawSize;

#if !UE_SERVER
	FWorldDelegates::LevelRemovedFromWorld.AddUObject(this, &UUnLive2DComponent::OnLevelRemovedFromWorld);

	if (IsRunningDedicatedServer()) return;

	const bool bIsGameWorld = GetWorld()->IsGameWorld();

	BodySetup = nullptr;

#if WITH_EDITOR
	if (!bIsGameWorld)
	{
		InitUnLive2D();
	}
#endif

#endif

}

void UUnLive2DComponent::OnUnregister()
{
#if !UE_SERVER
	FWorldDelegates::LevelRemovedFromWorld.RemoveAll(this);

#endif

	Super::OnUnregister();
}

void UUnLive2DComponent::DestroyComponent(bool bPromoteChildren /*= false*/)
{
	Super::DestroyComponent(bPromoteChildren);

	if (GetWorld() && !GetWorld()->IsEditorWorld())
	{
		ReleaseResources();
	}
}

FVector2D UUnLive2DComponent::ConvertPivotSpaceToTextureSpace(FVector2D Input) const
{
	const FVector2D Pivot = GetPivotPosition();


	const float X = Input.X + Pivot.X;
	const float Y = -Input.Y + Pivot.Y;

	return FVector2D(X, Y);
}

FVector2D UUnLive2DComponent::ConvertWorldSpaceToTextureSpace(const FVector& WorldPoint) const
{
	const FVector ProjectionX = WorldPoint.ProjectOnTo(UnLive2DAxisX);
	const FVector ProjectionY = WorldPoint.ProjectOnTo(UnLive2DAxisY);

	const float XValue = FMath::Sign(ProjectionX | UnLive2DAxisX) * ProjectionX.Size() ;
	const float YValue = FMath::Sign(ProjectionY | UnLive2DAxisY) * ProjectionY.Size() ;

	return ConvertPivotSpaceToTextureSpace(FVector2D(XValue, YValue));
}

FVector2D UUnLive2DComponent::GetRawPivotPosition() const
{
	FVector2D Dimension = SourceUnLive2D->DrawSize;
	FVector2D Pivot = SourceUnLive2D->Pivot;


	return Dimension * Pivot;
}

FVector2D UUnLive2DComponent::GetPivotPosition() const
{
	FVector2D RawPivot = GetRawPivotPosition();

	RawPivot.X = FMath::RoundToFloat(RawPivot.X);
	RawPivot.Y = FMath::RoundToFloat(RawPivot.Y);

	return RawPivot;
}

void UUnLive2DComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if !UE_SERVER
	if (!IsRunningDedicatedServer())
	{
		if (SourceUnLive2D == nullptr) return;

		UpdateUnLive2D();

		if (FUnLive2DRawModel* RawModel = SourceUnLive2D->GetUnLive2DRawModel().Pin().Get())
		{

			RawModel->OnUpDate(DeltaTime * SourceUnLive2D->PlayRate);

			const float DeltaTimeFromLastDraw = LastWidgetRenderTime == 0 ? 0 : (GetCurrentTime() - LastWidgetRenderTime);

			DrawWidgetToRenderTarget(DeltaTimeFromLastDraw);
		}

	}

#endif
}

const UObject* UUnLive2DComponent::AdditionalStatObject() const
{
	return SourceUnLive2D;
}

void UUnLive2DComponent::BeginPlay()
{
	InitUnLive2D();
	Super::BeginPlay();
	UpdateBodySetup(true);
	RecreatePhysicsState();
}

void UUnLive2DComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{

}

#undef LOCTEXT_NAMESPACE