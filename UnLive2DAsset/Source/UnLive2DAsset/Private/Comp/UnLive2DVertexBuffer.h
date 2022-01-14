#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "LocalVertexFactory.h"
#include "RenderResource.h"
#include "PackedNormal.h"
#include "SceneManagement.h"
#include "DynamicMeshBuilder.h"

class FUnLive2DVertexBuffer : public FVertexBuffer
{
public:
	//Buffers
	FVertexBuffer PositionBuffer;
	FVertexBuffer TangentBuffer;
	FVertexBuffer TexCoordBuffer;
	FVertexBuffer ColorBuffer;
	FIndexBuffer IndexBuffer;

	//SRVs for Manual Fetch on platforms that support it
	FShaderResourceViewRHIRef TangentBufferSRV;
	FShaderResourceViewRHIRef TexCoordBufferSRV;
	FShaderResourceViewRHIRef ColorBufferSRV;
	FShaderResourceViewRHIRef PositionBufferSRV;

	//Vertex data
	TArray<FDynamicMeshVertex> Vertices;

public:
	//Ctor
	FUnLive2DVertexBuffer()
		: bDynamicUsage(true)
		, NumAllocatedVertices(0)
	{}

public:
	void SetDynamicUsage(bool bInDynamicUsage);

	void CreateBuffers(int32 NumVertices);

	void ReleaseBuffers();

	void CommitVertexData();

	// FRenderResource interface
	virtual void InitRHI() override;
	virtual void ReleaseRHI() override;
	virtual void InitResource() override;
	virtual void ReleaseResource() override;
	// End of FRenderResource interface

	FORCEINLINE bool CommitRequiresBufferRecreation() const { return NumAllocatedVertices != Vertices.Num(); }

	/* Checks if the buffer has been initialized. */
	FORCEINLINE bool IsInitialized() const { return NumAllocatedVertices > 0; }

	/* Obtain the index buffer initialized for this buffer. */
	FORCEINLINE const FIndexBuffer* GetIndexPtr() const { return &IndexBuffer; }

private:
	/* Indicates if this buffer will be configured for dynamic usage. */
	bool bDynamicUsage;

	/* Amount of vertices allocated on the vertex buffer. */
	int32 NumAllocatedVertices;
};


class FUnLive2DVertexFactory : public FLocalVertexFactory
{
public:
	FUnLive2DVertexFactory(ERHIFeatureLevel::Type FeatureLevel);

	/* Initializes this factory with a given vertex buffer. */
	void Init(const FUnLive2DVertexBuffer* InVertexBuffer);

private:
	/* Vertex buffer used to initialize this factory. */
	const FUnLive2DVertexBuffer* VertexBuffer;
};