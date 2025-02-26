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
		StyleSet->Set(FName(TEXT("ClassThumbnail.UnLive2DMotion")), new IMAGE_BRUSH("AnimComposite_64x", FVector2D(64, 64)));
		StyleSet->Set(FName(TEXT("ClassIcon.UnLive2DMotion")), new IMAGE_BRUSH("AnimComposite_64x", FVector2D(16, 16)));
	}

	{
		StyleSet->Set(FName(TEXT("ClassThumbnail.UnLive2DAnimBlueprint")), new IMAGE_BRUSH("AnimInstance_64x", FVector2D(64, 64)));
		StyleSet->Set(FName(TEXT("ClassIcon.UnLive2DAnimBlueprint")), new IMAGE_BRUSH("AnimInstance_64x", FVector2D(16, 16)));
	}

	{
		StyleSet->Set("UnLive2DAnim.TabIcon", new IMAGE_BRUSH("Slate/UnLive2DAnimTab_16x", FVector2D(16, 16)));
		StyleSet->Set("UnLive2DAnim.RigUnit", new IMAGE_BRUSH("Slate/UnLive2DAnimUnit_16x", FVector2D(16, 16)));
	}

	{
		StyleSet->Set("UnLive2DAnimBlueprintGraphEditor.PlayUnLive2DAnimBlueprint", new IMAGE_BRUSH("Slate/icon_PlayAnim_40x", FVector2D(40, 40)));
		StyleSet->Set("UnLive2DAnimBlueprintGraphEditor.PlayUnLive2DAnimBlueprint.Small", new IMAGE_BRUSH("Slate/icon_PlayAnim_16x", FVector2D(16, 16)));
		StyleSet->Set("UnLive2DAnimBlueprintGraphEditor.PlayNode", new IMAGE_BRUSH("Slate/icon_PlayNode_40x", FVector2D(40, 40)));
		StyleSet->Set("UnLive2DAnimBlueprintGraphEditor.PlayNode.Small", new IMAGE_BRUSH("Slate/icon_PlayNode_40x", FVector2D(16, 16)));
		StyleSet->Set("UnLive2DAnimBlueprintGraphEditor.StopUnLive2DAnim", new IMAGE_BRUSH("Slate/icon_Stop_40x", FVector2D(40, 40)));
		StyleSet->Set("UnLive2DAnimBlueprintGraphEditor.StopUnLive2DAnim.Small", new IMAGE_BRUSH("Slate/icon_Stop_16x", FVector2D(16, 16)));
	}

	{
		StyleSet->Set("UnLive2DEditor.SetShowGrid", new IMAGE_BRUSH(TEXT("icon_MatEd_Grid_40x"), FVector2D(40, 40)));
		StyleSet->Set("UnLive2DEditor.SetShowGrid.Small", new IMAGE_BRUSH(TEXT("icon_MatEd_Grid_40x"), FVector2D(20, 20)));
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