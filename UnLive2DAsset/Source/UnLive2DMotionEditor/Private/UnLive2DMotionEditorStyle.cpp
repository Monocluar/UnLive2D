#include "UnLive2DMotionEditorStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr< FSlateStyleSet > FUnLive2DMotionEditorStyle::StyleSet = NULL;

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush(  StyleSet->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )

void FUnLive2DMotionEditorStyle::Initialize()
{
	if (StyleSet.IsValid())
	{
		return;
	}

	StyleSet = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	StyleSet->SetContentRoot(IPluginManager::Get().FindPlugin("UnLive2DAsset")->GetBaseDir() / TEXT("Resources"));

	{
		StyleSet->Set(FName(TEXT("ClassThumbnail.UnLive2DMotion")), new IMAGE_BRUSH("AnimComposite_64x", FVector2D(64, 64)));
		StyleSet->Set(FName(TEXT("ClassIcon.UnLive2DMotion")), new IMAGE_BRUSH("AnimComposite_16x", FVector2D(16, 16)));
	}


	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
}
#undef IMAGE_BRUSH

void FUnLive2DMotionEditorStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}

TSharedPtr< class ISlateStyle > FUnLive2DMotionEditorStyle::Get()
{
	return StyleSet;
}

FName FUnLive2DMotionEditorStyle::GetStyleSetName()
{
	return TEXT("UnLive2DMotionEditorStyle");
}

