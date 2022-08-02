#include "Animation/UnLive2DExpression.h"
#if WITH_EDITOR
#include "Misc/FileHelper.h"
#include "Motion/CubismExpressionMotion.hpp"
#include "CubismBpLib.h"
#include "CubismFramework.hpp"
#include "Id/CubismIdManager.hpp"
#include "UnLive2DRendererComponent.h"
#include "Id/CubismId.hpp"
#include "Serialization/JsonReader.h"
#include "Dom/JsonValue.h"
#endif
#include "Serialization/JsonSerializer.h"

#if WITH_EDITOR
using namespace Live2D::Cubism::Framework;

// 详细名称命名参考CubismExpressionMotion.cpp
const csmChar* ExpressionKeyParameters = "Parameters";
const csmChar* ExpressionKeyId = "Id";
const csmChar* ExpressionKeyValue = "Value";
const csmChar* ExpressionKeyBlend = "Blend";

const csmChar* BlendValueAdd = "Add";
const csmChar* BlendValueMultiply = "Multiply";
const csmChar* BlendValueOverwrite = "Overwrite";

bool UUnLive2DExpression::LoadLive2DExpressionData(const FString& ReadExpressionPath)
{
	const bool ReadSuc = FFileHelper::LoadFileToArray(ExpressionData.ByteData, *ReadExpressionPath);
	if (ReadSuc)
	{
		CubismExpressionMotion* Expression = CubismExpressionMotion::Create(ExpressionData.ByteData.GetData(), ExpressionData.ByteData.Num());
		FadeInTime = Expression->GetFadeInTime();
		FadeOutTime = Expression->GetFadeOutTime();


		CubismExpressionMotion::Delete(Expression);
	}

	return ReadSuc;

}

void UUnLive2DExpression::SetLive2DExpressionData(FUnLiveByteData& Data)
{
	ExpressionData = Data;

	CubismExpressionMotion* Expression = CubismExpressionMotion::Create(ExpressionData.ByteData.GetData(), ExpressionData.ByteData.Num());
	FadeInTime = Expression->GetFadeInTime();
	FadeOutTime = Expression->GetFadeOutTime();


	CubismExpressionMotion::Delete(Expression);
}

bool UUnLive2DExpression::GetAnimParamterGroup(TWeakObjectPtr<class UUnLive2DRendererComponent>& ObsComp, TArray<FUnLive2DParameterData_Expression>& ParameterArr)
{
	if (!ObsComp.IsValid() || !ObsComp->GetUnLive2DRawModel().IsValid()) return false;
	if (!ExpressionData.ByteData.IsValidIndex(0)) return false;
	ParameterArr.Empty();

	Utils::CubismJson* json = Utils::CubismJson::Create(ExpressionData.ByteData.GetData(), ExpressionData.ByteData.Num());
	Utils::Value& root = json->GetRoot();

	const csmInt32 parameterCount = root[ExpressionKeyParameters].GetSize();
	for (csmInt32 i = 0; i < parameterCount; ++i)
	{
		Utils::Value& param = root[ExpressionKeyParameters][i];
		const CubismIdHandle parameterId = CubismFramework::GetIdManager()->GetId(param[ExpressionKeyId].GetRawString()); // 参数标识
		const csmFloat32 value = static_cast<csmFloat32>(param[ExpressionKeyValue].ToFloat());   // 値
		FUnLive2DParameterData ParameterData;
		if (ObsComp->GetModelParamterIDData(param[ExpressionKeyId].GetRawString(), ParameterData))
		{
			ParameterData.ParameterValue = value;
		}

		EUnLive2DExpressionBlendType::Type BlendType;
		if (param[ExpressionKeyBlend].IsNull() || param[ExpressionKeyBlend].GetString() == BlendValueAdd)
		{
			BlendType = EUnLive2DExpressionBlendType::ExpressionBlendType_Add;
		}
		else if (param[ExpressionKeyBlend].GetString() == BlendValueMultiply)
		{
			BlendType = EUnLive2DExpressionBlendType::ExpressionBlendType_Multiply;
		}
		else if (param[ExpressionKeyBlend].GetString() == BlendValueOverwrite)
		{
			BlendType = EUnLive2DExpressionBlendType::ExpressionBlendType_Overwrite;
		}
		else
		{
			// 设置其他规格中没有的值时，通过设为加法模式恢复
			BlendType = EUnLive2DExpressionBlendType::ExpressionBlendType_Add;
		}
		ParameterArr.Add(FUnLive2DParameterData_Expression(ParameterData, BlendType));
	}

	return true;
}

void UUnLive2DExpression::SetAnimParamterValue(FName ParameterStr, float NewParameter)
{
	if (!ExpressionDataJsonRoot.IsValid())
	{
		FString JsonStr;
		FFileHelper::BufferToString(JsonStr, ExpressionData.ByteData.GetData(), ExpressionData.ByteData.Num());
		TSharedRef<TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(JsonStr);
		if (!FJsonSerializer::Deserialize(JsonReader, ExpressionDataJsonRoot)) return;

	}

	TArray<TSharedPtr<FJsonValue>> ParameterArr = ExpressionDataJsonRoot->GetArrayField(ExpressionKeyParameters);

	bool bIsHasParamter = false;
	for (TSharedPtr<FJsonValue>& Parameter : ParameterArr)
	{
		if (!Parameter.IsValid()) continue;

		TSharedPtr<FJsonObject> ParameterObject = Parameter->AsObject();

		if (ParameterObject->GetStringField(ExpressionKeyId) == ParameterStr.ToString())
		{
			bIsHasParamter = true;

			ParameterObject->SetNumberField(ExpressionKeyValue, NewParameter);
		}
	}

	if (bIsHasParamter)
	{
		FString OutJsonString;
		TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&OutJsonString, 0);
		if (FJsonSerializer::Serialize(ExpressionDataJsonRoot.ToSharedRef(), JsonWriter))
		{
			ExpressionData.ByteData.SetNum(OutJsonString.Len());
			FMemory::Memcpy(ExpressionData.ByteData.GetData(), TCHAR_TO_ANSI(*OutJsonString), OutJsonString.Len());
			UE_LOG(LogTemp, Log, TEXT(""));
		}

	}
}

void UUnLive2DExpression::SetAnimParamterBlendType(FName ParameterStr, float NewParameter)
{

}

#endif

const FUnLiveByteData* UUnLive2DExpression::GetExpressionData() const
{
	return &ExpressionData;
}
