#include "UnLive2DSceneProxy.h"
#include "UnLive2DComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "UnLive2D.h"

SIZE_T FUnLive2DSceneProxy::GetTypeHash() const
{
	static size_t UniquePointer;
	return reinterpret_cast<size_t>(&UniquePointer);
}

FUnLive2DSceneProxy::FUnLive2DSceneProxy(class UUnLive2DComponent* InComp)
	: FPrimitiveSceneProxy(InComp, InComp->GetUnLive2D()->GetFName())
	, RenderTarget(InComp->GetRenderTarget())
	, MaterialInstance(InComp->GetMaterialInstance())
	, BodySetup(InComp->GetBodySetup())
{
	Pivot = InComp->GetUnLive2D() ? InComp->GetUnLive2D()->Pivot : FVector2D(0.5f, 0.5f);
	MaterialRelevance = MaterialInstance->GetRelevance_Concurrent(GetScene().GetFeatureLevel());
}

void FUnLive2DSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, class FMeshElementCollector& Collector) const
{
#if WITH_EDITOR
	const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

	auto WireframeMaterialInstance = new FColoredMaterialRenderProxy(
		GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : nullptr,
		FLinearColor(0, 0.5f, 1.f)
	);

	Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);

	FMaterialRenderProxy* ParentMaterialProxy = nullptr;
	if (bWireframe)
	{
		ParentMaterialProxy = WireframeMaterialInstance;
	}
	else
	{
		ParentMaterialProxy = MaterialInstance->GetRenderProxy();
	}
#else
	FMaterialRenderProxy* ParentMaterialProxy = MaterialInstance->GetRenderProxy();
#endif

	const FMatrix& ViewportLocalToWorld = GetLocalToWorld(); // 世界矩阵

	FMatrix PreviousLocalToWorld; // 上一次绘制在世界的矩阵

	if (!GetScene().GetPreviousLocalToWorld(GetPrimitiveSceneInfo(), PreviousLocalToWorld))
	{
		PreviousLocalToWorld = GetLocalToWorld();
	}

	if (RenderTarget)
	{
		FTextureResource* TextureResource = RenderTarget->Resource;
		if (TextureResource)
		{
			float U = -RenderTarget->SizeX * Pivot.X;
			float V = -RenderTarget->SizeY * Pivot.Y;
			float UL = RenderTarget->SizeX * (1.0f - Pivot.X);
			float VL = RenderTarget->SizeY * (1.0f - Pivot.Y);

			int32 VertexIndices[4];

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				FDynamicMeshBuilder MeshBuilder(Views[ViewIndex]->GetFeatureLevel());

				if (VisibilityMap & (1 << ViewIndex))
				{
					VertexIndices[0] = MeshBuilder.AddVertex(-FVector(0, U, V), FVector2D(0, 0), FVector(0, -1, 0), FVector(0, 0, -1), FVector(1, 0, 0), FColor::White);
					VertexIndices[1] = MeshBuilder.AddVertex(-FVector(0, U, VL), FVector2D(0, 1), FVector(0, -1, 0), FVector(0, 0, -1), FVector(1, 0, 0), FColor::White);
					VertexIndices[2] = MeshBuilder.AddVertex(-FVector(0, UL, VL), FVector2D(1, 1), FVector(0, -1, 0), FVector(0, 0, -1), FVector(1, 0, 0), FColor::White);
					VertexIndices[3] = MeshBuilder.AddVertex(-FVector(0, UL, V), FVector2D(1, 0), FVector(0, -1, 0), FVector(0, 0, -1), FVector(1, 0, 0), FColor::White);

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

	}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		if (VisibilityMap & (1 << ViewIndex))
		{
			RenderCollision(BodySetup, Collector, ViewIndex, ViewFamily.EngineShowFlags, GetBounds(), IsSelected());
			RenderBounds(Collector.GetPDI(ViewIndex), ViewFamily.EngineShowFlags, GetBounds(), IsSelected());
		}
	}
#endif
}

void FUnLive2DSceneProxy::RenderCollision(UBodySetup* InBodySetup, FMeshElementCollector& Collector, int32 ViewIndex, const FEngineShowFlags& EngineShowFlags, const FBoxSphereBounds& InBounds, bool bRenderInEditor) const
{
	if (!InBodySetup) return;

	bool bDrawCollision = EngineShowFlags.Collision && IsCollisionEnabled();

	if (bDrawCollision && AllowDebugViewmodes())
	{
		// 如果“显示碰撞”，则将简单碰撞绘制为线框，启用了碰撞，并且我们不使用复杂的线框
		const bool bDrawSimpleWireframeCollision = InBodySetup->CollisionTraceFlag != ECollisionTraceFlag::CTF_UseComplexAsSimple;

		if (FMath::Abs(GetLocalToWorld().Determinant()) < SMALL_NUMBER)
		{
			//UE_LOG(LogLive2D, Log, TEXT("不支持零缩放 (%s)"), *StaticMesh->GetPathName());
		}
		else
		{
			const bool bDrawSolid = !bDrawSimpleWireframeCollision;
			const bool bProxyIsSelected = IsSelected();

			if (bDrawSolid)
			{
				// 制作用于绘制实体碰撞材质的材质
				auto SolidMaterialInstance = new FColoredMaterialRenderProxy(
					GEngine->ShadedLevelColorationUnlitMaterial->GetRenderProxy(),
					GetWireframeColor()
				);

				Collector.RegisterOneFrameMaterialProxy(SolidMaterialInstance);

				FTransform GeomTransform(GetLocalToWorld());
				InBodySetup->AggGeom.GetAggGeom(GeomTransform, GetWireframeColor().ToFColor(true), SolidMaterialInstance, false, true, DrawsVelocity(), ViewIndex, Collector);
			}
			// 线框
			else
			{
				FColor CollisionColor = FColor(157, 149, 223, 255);
				FTransform GeomTransform(GetLocalToWorld());
				InBodySetup->AggGeom.GetAggGeom(GeomTransform, GetSelectionColor(CollisionColor, bProxyIsSelected, IsHovered()).ToFColor(true), nullptr, false, false, DrawsVelocity(), ViewIndex, Collector);
			}
		}
	}
}

FPrimitiveViewRelevance FUnLive2DSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	bool bVisible = true;

	FPrimitiveViewRelevance Result;

	MaterialRelevance.SetPrimitiveViewRelevance(Result);

	Result.bDrawRelevance = IsShown(View) && bVisible && View->Family->EngineShowFlags.WidgetComponents;
	Result.bDynamicRelevance = true;
	Result.bRenderCustomDepth = ShouldRenderCustomDepth();
	Result.bRenderInMainPass = ShouldRenderInMainPass();
	Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
	Result.bShadowRelevance = IsShadowCast(View);
	Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
	Result.bEditorPrimitiveRelevance = false;
	Result.bVelocityRelevance = IsMovable() && Result.bOpaque && Result.bRenderInMainPass;

	return Result;
}

/*
void FUnLive2DSceneProxy::GetLightRelevance(const FLightSceneProxy* LightSceneProxy, bool& bDynamic, bool& bRelevant, bool& bLightMapped, bool& bShadowMapped) const
{
	bDynamic = false;
	bRelevant = false;
	bLightMapped = false;
	bShadowMapped = false;
}*/

void FUnLive2DSceneProxy::OnTransformChanged()
{
	Origin = GetLocalToWorld().GetOrigin();
}

bool FUnLive2DSceneProxy::CanBeOccluded() const
{
	return !MaterialRelevance.bDisableDepthTest;
}

uint32 FUnLive2DSceneProxy::GetMemoryFootprint(void) const
{
	return(sizeof(*this) + GetAllocatedSize());
}

FORCEINLINE uint32 FUnLive2DSceneProxy::GetAllocatedSize(void) const
{
	return FPrimitiveSceneProxy::GetAllocatedSize() ; 
}

