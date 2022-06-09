#pragma once

#include "CoreMinimal.h"
#include "CubismConfig.h"
#include "UnLive2DAnimBase.h"
#include "UnLive2DExpression.generated.h"

UCLASS(Blueprintable, BlueprintType, Category = UnLive2DAnim, hidecategories = Object)
class UNLIVE2DASSET_API UUnLive2DExpression : public UUnLive2DAnimBase
{
	GENERATED_BODY()

#if WITH_EDITOR
public:
	bool LoadLive2DExpressionData(const FString& ReadExpressionPath);

	void SetLive2DExpressionData(FUnLiveByteData& Data);

#endif

public:

	const FUnLiveByteData* GetExpressionData() const;

protected:
	// 表情数据
	UPROPERTY()
		FUnLiveByteData ExpressionData;
};