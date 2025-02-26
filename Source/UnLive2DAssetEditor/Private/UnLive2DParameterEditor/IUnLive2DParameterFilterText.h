
#pragma once

#include "CoreMinimal.h"
#include "SUnLive2DParameterInfo.h"

class IUnLive2DParameterFilterText
{
public:

	virtual FText& GetFilterText() = 0;

	virtual void OnNameCommitted(const FText& NewName, ETextCommit::Type CommitType, TSharedPtr<FUnLive2DParameterInfo> Item) = 0;
};