#include "UnLive2DManagerToolkit.h"

FUnLive2DMangerToolkit::FUnLive2DMangerToolkit()
	: UnLive2D(nullptr)
{

}

static void FindCounterpartAssets(const UObject* InAsset, TWeakObjectPtr<UUnLive2D>& OutUnLive2D)
{
	const UUnLive2D* CounterpartLive2D = OutUnLive2D.Get();
	FUnLive2DManagerAssetFamily::FindCounterpartAssets(InAsset, CounterpartLive2D);
	OutUnLive2D = MakeWeakObjectPtr(const_cast<UUnLive2D*>(CounterpartLive2D));
}

void FUnLive2DMangerToolkit::Initialize(UUnLive2D* InUnLive2D)
{
	check(InUnLive2D);
	UnLive2D = InUnLive2D;
	InitialAssetClass = UUnLive2D::StaticClass();

	FindCounterpartAssets(InUnLive2D, UnLive2D);
}

void FUnLive2DMangerToolkit::CreatePreviewScene()
{

}

UUnLive2D* FUnLive2DMangerToolkit::GetUnLive2D() const
{
	return UnLive2D.Get();
}

int32 FUnLive2DMangerToolkit::GetCustomData(const int32 Key) const
{
	if (!CustomEditorData.Contains(Key))
	{
		return INDEX_NONE;
	}
	return CustomEditorData[Key];
}

void FUnLive2DMangerToolkit::SetCustomData(const int32 Key, const int32 CustomData)
{
	CustomEditorData.FindOrAdd(Key) = CustomData;
}

FName FUnLive2DMangerToolkit::GetContext() const
{
	if (InitialAssetClass != nullptr)
	{
		return InitialAssetClass->GetFName();
	}

	return NAME_None;
}



