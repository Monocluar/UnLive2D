#include "Draw/UnLive2DTargetBoxProxy.h"
#include "UnLive2DRendererComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "FWPort/UnLive2DRawModel.h"
#include "FWPort/UnLive2DModelRender.h"
#include "Engine/TextureRenderTarget2D.h"
#include "DynamicMeshBuilder.h"
#include "RHICommandList.h"
#include "GlobalShader.h"
#include "UnLive2DSetting.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "MaterialDomain.h"
#include "Materials/MaterialRenderProxy.h"
#endif
#include "Kismet/KismetSystemLibrary.h"
#include "ShaderParameterUtils.h"
#include "PhysicsEngine/BodySetup.h"

void FUnLive2DTargetBoxProxy::UpdateSection_RenderThread(FRHICommandListImmediate& RHICmdList)
{
	if (!UnLive2DRawModel.IsValid()) return;

	if (RenderTarget)
	{
		if (UnLive2DClippingManager.IsValid())
		{
			bool bNoLowPreciseMask = false;
			UnLive2DClippingManager->SetupClippingContext(bNoLowPreciseMask);
			// 先绘制遮罩Buffer
			UnLive2DClippingManager->RenderMask_Full(RHICmdList, GetScene().GetFeatureLevel(), MaskBuffer);
			//UnLive2DClippingManager->RenderMask_Full(RHICmdList, GetScene().GetFeatureLevel(), RenderTarget->GetRenderTargetResource()->TextureRHI);
		}
		DrawSeparateToRenderTarget_RenderThread(RHICmdList, RenderTarget->GetRenderTargetResource(),GetScene().GetFeatureLevel(), MaskBuffer);
	}
}

void FUnLive2DTargetBoxProxy::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials /*= false*/) const
{
	OutMaterials.Add(MaterialInstance);
}

bool FUnLive2DTargetBoxProxy::OnUpData()
{
	return UpdataRTSections(bCombinedbBatch);
}

void FUnLive2DTargetBoxProxy::OnUpDataRenderer()
{
	ENQUEUE_RENDER_COMMAND(UnLive2DTargetBoxProxy_OnUpData)([this](FRHICommandListImmediate& RHICmdList)
		{
			UpdateSection_RenderThread(RHICmdList);
		});
}


SIZE_T FUnLive2DTargetBoxProxy::GetTypeHash() const
{
	static size_t UniquePointer;
	return reinterpret_cast<size_t>(&UniquePointer);
}

uint32 FUnLive2DTargetBoxProxy::GetMemoryFootprint() const
{
	return sizeof(*this) + FPrimitiveSceneProxy::GetAllocatedSize();
}

bool FUnLive2DTargetBoxProxy::CanBeOccluded() const
{
	return !MaterialRelevance.bDisableDepthTest;
}

void FUnLive2DTargetBoxProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
{
	if (MaterialInstance == nullptr || RenderTarget == nullptr) return;
	FMaterialRenderProxy* ParentMaterialProxy = MaterialInstance->GetRenderProxy();

	const FMatrix& ViewportLocalToWorld = GetLocalToWorld();

	FMatrix PreviousLocalToWorld;

	if (!GetScene().GetPreviousLocalToWorld(GetPrimitiveSceneInfo(), PreviousLocalToWorld))
	{
		PreviousLocalToWorld = GetLocalToWorld();
	}

	FTextureResource* TextureResource = RenderTarget->GetResource();
	if (TextureResource)
	{
		float U = -CurrentDrawSize.X;
		float V = -CurrentDrawSize.Y;
		float UL = CurrentDrawSize.X;
		float VL = CurrentDrawSize.Y;

		int32 VertexIndices[4];

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			FDynamicMeshBuilder MeshBuilder(Views[ViewIndex]->GetFeatureLevel());

			if (VisibilityMap & (1 << ViewIndex))
			{
				VertexIndices[0] = MeshBuilder.AddVertex(-FULVector3f(0, U, V), FULVector2f(0, 0), FULVector3f(0, -1, 0), FULVector3f(0, 0, -1), FULVector3f(1, 0, 0), FColor::White);
				VertexIndices[1] = MeshBuilder.AddVertex(-FULVector3f(0, U, VL), FULVector2f(0, 1), FULVector3f(0, -1, 0), FULVector3f(0, 0, -1), FULVector3f(1, 0, 0), FColor::White);
				VertexIndices[2] = MeshBuilder.AddVertex(-FULVector3f(0, UL, VL), FULVector2f(1, 1), FULVector3f(0, -1, 0), FULVector3f(0, 0, -1), FULVector3f(1, 0, 0), FColor::White);
				VertexIndices[3] = MeshBuilder.AddVertex(-FULVector3f(0, UL, V), FULVector2f(1, 0), FULVector3f(0, -1, 0), FULVector3f(0, 0, -1), FULVector3f(1, 0, 0), FColor::White);

				MeshBuilder.AddTriangle(VertexIndices[0], VertexIndices[1], VertexIndices[2]);
				MeshBuilder.AddTriangle(VertexIndices[0], VertexIndices[2], VertexIndices[3]);

				FDynamicMeshBuilderSettings Settings;
				Settings.bDisableBackfaceCulling = false;
				Settings.bReceivesDecals = true;
				Settings.bUseSelectionOutline = true;
				MeshBuilder.GetMesh(ViewportLocalToWorld, PreviousLocalToWorld, ParentMaterialProxy, SDPG_World, Settings, nullptr, ViewIndex, Collector, FHitProxyId());
			}
		}
	}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	/*for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		if (VisibilityMap & (1 << ViewIndex))
		{
			RenderCollision(BodySetup, Collector, ViewIndex, ViewFamily.EngineShowFlags, GetBounds(), IsSelected());
			RenderBounds(Collector.GetPDI(ViewIndex), ViewFamily.EngineShowFlags, GetBounds(), IsSelected());
		}
	}*/
#endif
}

FPrimitiveViewRelevance FUnLive2DTargetBoxProxy::GetViewRelevance(const FSceneView* View) const
{

	FPrimitiveViewRelevance Result;

	MaterialRelevance.SetPrimitiveViewRelevance(Result);

	Result.bDrawRelevance = IsShown(View) && View->Family->EngineShowFlags.Paper2DSprites; // 使用Sprites的
	Result.bDynamicRelevance = true;
	Result.bRenderCustomDepth = ShouldRenderCustomDepth();
	Result.bRenderInMainPass = ShouldRenderInMainPass();
	Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
	Result.bShadowRelevance = IsShadowCast(View);
	Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
	Result.bEditorPrimitiveRelevance = false;
	Result.bVelocityRelevance = DrawsVelocity() && Result.bOpaque && Result.bRenderInMainPass;

	return Result;
}

/*
void FUnLive2DTargetBoxProxy::GetLightRelevance(const FLightSceneProxy* LightSceneProxy, bool& bDynamic, bool& bRelevant, bool& bLightMapped, bool& bShadowMapped) const
{
	bDynamic = false;
	bRelevant = false;
	bLightMapped = false;
	bShadowMapped = false;
}*/

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
void FUnLive2DTargetBoxProxy::RenderCollision(UBodySetup* InBodySetup, FMeshElementCollector& Collector, int32 ViewIndex, const FEngineShowFlags& EngineShowFlags, const FBoxSphereBounds& InBounds, bool bRenderInEditor) const
{

	if (InBodySetup)
	{
		bool bDrawCollision = EngineShowFlags.Collision && IsCollisionEnabled();

		if (bDrawCollision && AllowDebugViewmodes())
		{
			const bool bDrawSimpleWireframeCollision = InBodySetup->CollisionTraceFlag != ECollisionTraceFlag::CTF_UseComplexAsSimple;

			if (FMath::Abs(GetLocalToWorld().Determinant()) < SMALL_NUMBER) return;

			{
				const bool bDrawSolid = !bDrawSimpleWireframeCollision;
				const bool bProxyIsSelected = IsSelected();

				if (bDrawSolid)
				{
					// Make a material for drawing solid collision stuff
					auto SolidMaterialInstance = new FColoredMaterialRenderProxy(
						GEngine->ShadedLevelColorationUnlitMaterial->GetRenderProxy(),
						GetWireframeColor()
					);

					Collector.RegisterOneFrameMaterialProxy(SolidMaterialInstance);

					FTransform GeomTransform(GetLocalToWorld());
					InBodySetup->AggGeom.GetAggGeom(GeomTransform, GetWireframeColor().ToFColor(true), SolidMaterialInstance, false, true, DrawsVelocity(), ViewIndex, Collector);
				}
				else
				{
					FColor CollisionColor = FColor(157, 149, 223, 255);
					FTransform GeomTransform(GetLocalToWorld());
					InBodySetup->AggGeom.GetAggGeom(GeomTransform, GetSelectionColor(CollisionColor, bProxyIsSelected, IsHovered()).ToFColor(true), nullptr, false, false, DrawsVelocity(), ViewIndex, Collector);
				}
			}
		}
	}
}
#endif

#if WITH_EDITOR
void FUnLive2DTargetBoxProxy::UpDataUnLive2DProperty(FName PropertyName)
{
	if (PropertyName == TEXT("TintColorAndOpacity"))
	{
		MaterialInstance->SetVectorParameterValue("TintColorAndOpacity", OwnerComponent->GetUnLive2D()->TintColorAndOpacity);
	}
}
#endif

const UTexture2D* FUnLive2DTargetBoxProxy::GetTexture(const uint8& TextureIndex) const
{
	if (OwnerComponent == nullptr || OwnerComponent->GetUnLive2D() == nullptr || !OwnerComponent->GetUnLive2D()->TextureAssets.IsValidIndex(TextureIndex)) return nullptr;

	return OwnerComponent->GetUnLive2D()->TextureAssets[TextureIndex];
}

FUnLive2DTargetBoxProxy::FUnLive2DTargetBoxProxy(UUnLive2DRendererComponent* InComponent)
	: UnLive2DProxyBase(InComponent)
	, bCombinedbBatch(true)
	, BodySetup(InComponent->GetBodySetup())
{
	UnLive2DRawModel = InComponent->GetUnLive2DRawModel().Pin();
	if (!UnLive2DRawModel.IsValid()) return;
	Csm::CubismModel* UnLive2DModel = UnLive2DRawModel->GetModel();
	if (UnLive2DModel == nullptr) return;


	if (CreateClippingManager())
	{
		ETextureCreateFlags Flags = ETextureCreateFlags(TexCreate_None | TexCreate_RenderTargetable | TexCreate_ShaderResource);
		const csmInt32 BufferHeight = UnLive2DClippingManager->GetClippingMaskBufferSize();
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

	{
		csmFloat32 CanvasWidth = UnLive2DModel->GetCanvasWidth();
		csmFloat32 CanvasHeight = UnLive2DModel->GetCanvasHeight();
		CurrentDrawSize = FIntPoint(Live2DScale, Live2DScale);

		const FVector Origin = FVector(.5f,
			-(CurrentDrawSize.X) + (CurrentDrawSize.X),
			-(CurrentDrawSize.Y) + (CurrentDrawSize.Y));

		const FVector BoxExtent = FVector(1.f, CurrentDrawSize.X / 2.0f, CurrentDrawSize.Y / 2.0f);

		LocalBox = FBoxSphereBounds(Origin, BoxExtent, CurrentDrawSize.Size() / 2.0f);
	}


	RenderTarget = NewObject<UTextureRenderTarget2D>(GetTransientPackage());
	RenderTarget->ClearColor = FLinearColor(0.f, 0.f, 0.f, 0.f);
	const EPixelFormat RequestedFormat = FSlateApplication::Get().GetRenderer()->GetSlateRecommendedColorFormat();
	RenderTarget->InitCustomFormat(InComponent->RenderTargetSize, InComponent->RenderTargetSize, RequestedFormat, false);
	RenderTarget->AddToRoot();

	// 使用3DUI的
	if (InComponent->UnLive2DRTMaterial.IsNull()) return;
	UMaterialInterface* MaterialInterface = Cast<UMaterialInterface>(InComponent->UnLive2DRTMaterial.TryLoad());
	if (MaterialInterface == nullptr) return;
	
	UMaterialInstanceDynamic* MaterialDynamic = UMaterialInstanceDynamic::Create(MaterialInterface, InComponent);
	if (MaterialDynamic)
	{
		MaterialRelevance = MaterialDynamic->GetRelevance_Concurrent(GetScene().GetFeatureLevel());
		MaterialDynamic->AddToCluster(InComponent);
		MaterialDynamic->SetTextureParameterValue("SlateUI", RenderTarget);
#if WITH_EDITOR
#if ENGINE_MAJOR_VERSION < 5 || ENGINE_MINOR_VERSION <= 4
		ENQUEUE_RENDER_COMMAND(FMeshRenderBufferSetDestroy)(
			[this, MaterialDynamic](FRHICommandListImmediate& RHICmdList)
			{
#endif
				TArray<UMaterialInterface*> InUsedMaterialsForVerification;
				InUsedMaterialsForVerification.Add(MaterialDynamic);
				this->SetUsedMaterialForVerification(InUsedMaterialsForVerification);

#if ENGINE_MAJOR_VERSION < 5 || ENGINE_MINOR_VERSION <= 4
			});
#endif
#endif
		MaterialDynamic->SetVectorParameterValue("TintColorAndOpacity", InComponent->GetUnLive2D()->TintColorAndOpacity);
		//MaterialDynamic->SetScalarParameterValue("OpacityFromTexture", OpacityFromTexture);
		MaterialInstance = MaterialDynamic;
		MaterialInstance->AddToRoot();
	}

	//MaterialInstance = UMaterial::GetDefaultMaterial(MD_Surface);
}

FUnLive2DTargetBoxProxy::~FUnLive2DTargetBoxProxy()
{
	if (IsValid(RenderTarget))
	{
		RenderTarget->RemoveFromRoot();
		RenderTarget = nullptr;
	}
	UnLive2DClippingManager.Reset();
	if (IsValid(MaterialInstance))
	{
		MaterialInstance->RemoveFromRoot();
	}
	MaterialInstance = nullptr;
	MaskBuffer.SafeRelease();
}