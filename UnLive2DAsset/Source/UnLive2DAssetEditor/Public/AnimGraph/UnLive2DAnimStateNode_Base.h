// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraphNode.h"
#include "UnLive2DAnimStateNode_Base.generated.h"

UCLASS(MinimalAPI, Abstract)
class UUnLive2DAnimStateNode_Base : public UEdGraphNode
{
	GENERATED_UCLASS_BODY()

protected:

	virtual void Serialize(FArchive& Ar) override;
	virtual void PostLoad() override;
};