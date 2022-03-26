
#pragma once

#include "RigVMCore/RigVMStruct.h"
#include "RigVMCore/RigVMRegistry.h"
#include "UnLive2DRigUnit.generated.h"


USTRUCT(BlueprintType, meta = (Abstract, NodeColor = "0.1 0.1 0.1"))
struct UNLIVE2DASSET_API FUnLive2DRigUnit : public FRigVMStruct
{
	GENERATED_BODY()

public:

	FUnLive2DRigUnit(){}

	virtual ~FUnLive2DRigUnit() {}

};
