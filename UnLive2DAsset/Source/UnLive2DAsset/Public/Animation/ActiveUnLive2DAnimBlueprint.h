
#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "UnLive2DAnimBlueprint.h"

struct UNLIVE2DASSET_API FActiveUnLive2DAnimBlueprint
{
public:
	FActiveUnLive2DAnimBlueprint();

	virtual ~FActiveUnLive2DAnimBlueprint();

public:
	virtual uint32 GetObjectId() const;

private:
	TWeakObjectPtr<UWorld> World;
	uint32 WorldID;
	TWeakObjectPtr<UUnLive2DAnimBlueprint> UnLive2DAnimBlueprint;

};