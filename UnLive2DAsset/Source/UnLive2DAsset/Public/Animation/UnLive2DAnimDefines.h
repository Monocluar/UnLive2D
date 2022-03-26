
#pragma once

#include "PropertyPathHelpers.h"
#include "RigVMCore/RigVMExecuteContext.h"
#include "Rigs/UnLive2DAnimRigHierarchyContainer.h"
#include "UnLive2DAnimDefines.generated.h"

USTRUCT()
struct UNLIVE2DASSET_API FUnLive2DAnimeExecuteContext : public FRigVMExecuteContext
{
	GENERATED_BODY()

public:
	FUnLive2DAnimeExecuteContext() 
		: FRigVMExecuteContext()
		, Hierarchy(nullptr)
	{}

public:
	FUnLive2DAnimRigHierarchyContainer* Hierarchy;
};