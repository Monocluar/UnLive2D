#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "RenderResource.h"


#if ENGINE_MAJOR_VERSION < 5

typedef FMatrix FUnLiveMatrix;
typedef FVector4 FUnLiveVector4;
typedef FVector2D FULVector2f;
typedef FVector FULVector3f;
typedef FVertexBufferRHIRef FUVBufferRHIRef;
typedef FIndexBufferRHIRef FUIBufferRHIRef;
typedef FBoxSphereBounds FULBoxSphereBounds;
#else
typedef FMatrix44f FUnLiveMatrix;
typedef FVector4f FUnLiveVector4;
typedef FVector2f FULVector2f;
typedef FVector3f FULVector3f;
typedef FBufferRHIRef FUVBufferRHIRef;
typedef FBufferRHIRef FUIBufferRHIRef;
typedef FBoxSphereBounds3f FULBoxSphereBounds;
#endif

#if WITH_EDITOR
#if ENGINE_MAJOR_VERSION < 5
typedef FEditorStyle FUnLive2DStyle;
#else
typedef FAppStyle FUnLive2DStyle;
#endif
class FUnLive2DAppStyle
{
public:
	static const FName GetStyleSetName()
	{
#if ENGINE_MAJOR_VERSION < 5
		return FEditorStyle::GetStyleSetName();
#else
		return FAppStyle::GetAppStyleSetName();
#endif
	}
};

#endif