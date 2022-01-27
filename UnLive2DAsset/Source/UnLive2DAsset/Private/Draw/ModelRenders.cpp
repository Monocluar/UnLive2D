#include "ModelRenders.h"

void FCubismVertexDeclaration::InitRHI()
{
	FVertexDeclarationElementList Elements;
	uint32 Stride = sizeof(FCubismVertex);
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FCubismVertex, Position), VET_Float2, 0, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FCubismVertex, UV), VET_Float2, 1, Stride));
	VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
}

void FCubismVertexDeclaration::ReleaseRHI()
{
	 VertexDeclarationRHI.SafeRelease();
}

TGlobalResource<FCubismVertexDeclaration> GCubismVertexDeclaration;