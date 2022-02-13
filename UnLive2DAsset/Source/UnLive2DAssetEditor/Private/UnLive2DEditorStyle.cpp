#include "UnLive2DEditorStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr< FSlateStyleSet > FUnLive2DEditorStyle::StyleSet = NULL;

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush(  StyleSet->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )

void FUnLive2DEditorStyle::Initialize()
{
    if (StyleSet.IsValid())
	{
		return;
	}

	StyleSet = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	StyleSet->SetContentRoot(IPluginManager::Get().FindPlugin("UnLive2DAsset")->GetBaseDir() / TEXT("Resources"));

	{

		StyleSet->Set(FName(TEXT("ClassThumbnail.UnLive2D")), new IMAGE_BRUSH("Icon128", FVector2D(64, 64)));
		StyleSet->Set(FName(TEXT("ClassIcon.UnLive2D")), new IMAGE_BRUSH("Icon40", FVector2D(16, 16)));

	}

	{
		StyleSet->Set(FName(TEXT("UnLive2DEditor.EnterViewMode")), new IMAGE_BRUSH("UnLive2D/Designer_40x", FVector2D(40, 40)));
		StyleSet->Set(FName(TEXT("UnLive2DEditor.EnterViewMode.Small")), new IMAGE_BRUSH("UnLive2D/Designer_16x", FVector2D(16, 16)));
		StyleSet->Set(FName(TEXT("UnLive2DEditor.EnterAnimMode")), new IMAGE_BRUSH("UnLive2D/Animations_40x", FVector2D(40, 40)));
		StyleSet->Set(FName(TEXT("UnLive2DEditor.EnterAnimMode.Small")), new IMAGE_BRUSH("UnLive2D/Animations_16x", FVector2D(16, 16)));
	}

    FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
}
#undef IMAGE_BRUSH

void FUnLive2DEditorStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}

FName FUnLive2DEditorStyle::GetStyleSetName()
{
	return TEXT("UnLive2DEditorStyle");
}

TSharedPtr< class ISlateStyle > FUnLive2DEditorStyle::Get()
{
	return StyleSet;
}