
#pragma once

#include "CoreMinimal.h"
#include "Stats/Stats.h"
#include "Widgets/SWidget.h"
#include "EdGraph/EdGraphPin.h"
#include "BlueprintEditor.h"

class UUnLive2DRendererComponent;

struct FUnLive2DAnimBlueprintEditorModes
{
	// Mode constants
	static const FName AnimationBlueprintEditorMode;
	static const FName AnimationBlueprintInterfaceEditorMode;

	static FText GetLocalizedMode(const FName InMode)
	{
		static TMap< FName, FText > LocModes;

		if (LocModes.Num() == 0)
		{
			LocModes.Add(AnimationBlueprintEditorMode, NSLOCTEXT("AnimationBlueprintEditorModes", "AnimationBlueprintEditorMode", "Animation Blueprint"));
			LocModes.Add(AnimationBlueprintInterfaceEditorMode, NSLOCTEXT("AnimationBlueprintEditorModes", "AnimationBlueprintInterface EditorMode", "Animation Blueprint Interface"));
		}

		check(InMode != NAME_None);
		const FText* OutDesc = LocModes.Find(InMode);
		check(OutDesc);
		return *OutDesc;
	}

private:
	FUnLive2DAnimBlueprintEditorModes() {}
};

namespace UnLive2DAnimationBlueprintEditorTabs
{
	extern const FName DetailsTab;
	extern const FName SkeletonTreeTab;
	extern const FName ViewportTab;
	extern const FName AdvancedPreviewTab;
	extern const FName AssetBrowserTab;
	extern const FName AnimBlueprintPreviewEditorTab;
	extern const FName AssetOverridesTab;
	extern const FName SlotNamesTab;
	extern const FName CurveNamesTab;
};

class FUnLive2DAnimationBlueprintEditor : public FBlueprintEditor
{
public:

	void InitUnLive2DAnimationBlueprintEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, class UUnLive2DAnimBlueprint* InAnimBlueprint);

public:
	FUnLive2DAnimationBlueprintEditor();

	virtual ~FUnLive2DAnimationBlueprintEditor();


private:

	void BindCommands();

private:

	/** Extend menu */
	void ExtendMenu();

	/** Extend toolbar */
	void ExtendToolbar();

	/** Called immediately prior to a blueprint compilation */
	void OnBlueprintPreCompile(UBlueprint* BlueprintToCompile);

private:

	TSharedPtr<class IUnLive2DToolkit> UnLive2DManagerToolkit;

	/** The extender to pass to the level editor to extend it's window menu */
	TSharedPtr<FExtender> MenuExtender;

	/** Toolbar extender */
	TSharedPtr<FExtender> ToolbarExtender;

	/** The last pin type we added to a graph's inputs */
	FEdGraphPinType LastGraphPinType;

	UUnLive2DRendererComponent* DebuggedUnLive2DComponent;
};