#include "Physics/UnLive2DPhysics.h"
#if WITH_EDITOR
#include "Physics/CubismPhysics.hpp"
#include "Misc/FileHelper.h"
#include "FWPort/UnLive2DRawModel.h"
#endif

UUnLive2DPhysics::UUnLive2DPhysics(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{ }


#if WITH_EDITOR
using namespace Live2D::Cubism::Framework;
bool UUnLive2DPhysics::LoadLive2DPhysicsData(const FString& ReadPhysicsPath)
{
	const bool ReadSuc = FFileHelper::LoadFileToArray(Live2DPhysicsData, *ReadPhysicsPath);

	if (ReadSuc)
	{
		CubismPhysics* Physics = CubismPhysics::Create(Live2DPhysicsData.GetData(), Live2DPhysicsData.Num()); // 解析
		DefaultGravity = FVector2D(Physics->GetOptions().Gravity.X, Physics->GetOptions().Gravity.Y);
		DefaultWind = FVector2D(Physics->GetOptions().Wind.X, Physics->GetOptions().Wind.Y);
		CubismPhysics::Delete(Physics);
	}
	return ReadSuc;
}

void UUnLive2DPhysics::SetLive2DPhysicsData(TArray<uint8>& InPhysicsData)
{
	Live2DPhysicsData = InPhysicsData;
	CubismPhysics* Physics = CubismPhysics::Create(Live2DPhysicsData.GetData(), Live2DPhysicsData.Num()); // 解析
	if (Physics)
	{
		DefaultGravity = FVector2D(Physics->GetOptions().Gravity.X, Physics->GetOptions().Gravity.Y);
		DefaultWind = FVector2D(Physics->GetOptions().Wind.X, Physics->GetOptions().Wind.Y);
	}
	CubismPhysics::Delete(Physics);
}

void UUnLive2DPhysics::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{	
	if (!PreviewLive2DRawModel.IsValid()) return;
	const FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;
	if (MemberPropertyName == GET_MEMBER_NAME_STRING_CHECKED(UUnLive2DPhysics, DefaultGravity))
	{
		PreviewLive2DRawModel.Pin()->SetPhysicsGravity(DefaultGravity);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_STRING_CHECKED(UUnLive2DPhysics, DefaultWind))
	{
		PreviewLive2DRawModel.Pin()->SetPhysicsWind(DefaultWind);
	}
}
#endif
