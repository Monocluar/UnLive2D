#include "Animation/UnLive2DExpression.h"
#if WITH_EDITOR
#include "Misc/FileHelper.h"
#endif

#if WITH_EDITOR
bool UUnLive2DExpression::LoadLive2DExpressionData(const FString& ReadExpressionPath)
{
	const bool ReadSuc = FFileHelper::LoadFileToArray(ExpressionData.ByteData, *ReadExpressionPath);

	return ReadSuc;

}

void UUnLive2DExpression::SetLive2DExpressionData(FUnLiveByteData& Data)
{
	ExpressionData = Data;
}

#endif

const FUnLiveByteData* UUnLive2DExpression::GetExpressionData() const
{
	return &ExpressionData;
}
