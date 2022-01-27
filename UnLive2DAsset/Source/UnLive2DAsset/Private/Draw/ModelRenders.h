#include "CubismFramework.hpp"
#include "Model/CubismModel.hpp"
#include "Type/CubismBasicType.hpp"
#include "RHIResources.h"
#include "RenderResource.h"
#include "RHICommandList.h"
#include "RHIDefinitions.h"
//////////////////////////////////////////////////////////////////////////
// See CommonRenderResources.cpp

/** The vertex data used to filter a texture. */
struct FCubismVertex
{
    FVector2D Position;
    FVector2D UV;

    FCubismVertex(float x, float y, float z, float w)
        : Position(x, y)
        , UV(z, w)
    {}
};

/** The filter vertex declaration resource type. */
class FCubismVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	/** Destructor. */
	virtual ~FCubismVertexDeclaration() {}

	virtual void InitRHI();

	virtual void ReleaseRHI();
};

extern TGlobalResource<FCubismVertexDeclaration> GCubismVertexDeclaration;
