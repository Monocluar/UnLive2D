#include "UnLive2DAnimBlueprintEditorMode.h"
#include "UnLive2DManagerModule.h"
#include "Animation/UnLive2DAnimBlueprint.h"
#include "UnLive2DAnimationBlueprintEditor.h"
#include "Framework/Docking/LayoutExtender.h"
#include "BlueprintEditorTabs.h"

FUnLive2DAnimBlueprintEditorMode::FUnLive2DAnimBlueprintEditorMode(const TSharedRef<FUnLive2DAnimationBlueprintEditor>& InAnimationBlueprintEditor)
	: FBlueprintEditorApplicationMode(InAnimationBlueprintEditor, FUnLive2DAnimBlueprintEditorModes::AnimationBlueprintEditorMode, FUnLive2DAnimBlueprintEditorModes::GetLocalizedMode, false, false)
{
	//PreviewScenePtr = InAnimationBlueprintEditor->GetPreviewScene();
	AnimBlueprintPtr =  CastChecked<UUnLive2DAnimBlueprint>(InAnimationBlueprintEditor->GetBlueprintObj());

	TabLayout = FTabManager::NewLayout( "Stanalone_UnLive2DAnimBlueprintEditorMode_Layout_v1.0" )
		->AddArea
		(
			{
				FTabManager::NewPrimaryArea()
				->SetOrientation(Orient_Vertical)
				->Split
				(
					// Top toolbar
					FTabManager::NewStack()
					->SetSizeCoefficient(0.186721f)
					->SetHideTabWell(true)
					->AddTab(InAnimationBlueprintEditor->GetToolbarTabId(), ETabState::OpenedTab)
				)
				->Split
				(
					{
						// Main application area
						FTabManager::NewSplitter()
						->SetOrientation(Orient_Horizontal)
						->Split
						(
							// Left top - viewport
							{
								FTabManager::NewSplitter()
								->SetSizeCoefficient(0.25f)
								->SetOrientation(Orient_Vertical)
								->Split
								(
									{
										// Left top - viewport
										FTabManager::NewStack()
										->SetSizeCoefficient(0.5f)
										->SetHideTabWell(true)
										->AddTab(UnLive2DAnimationBlueprintEditorTabs::ViewportTab, ETabState::OpenedTab)
									}
								)
								->Split
								(
									{
										//	Left bottom - preview settings
										FTabManager::NewStack()
										->SetSizeCoefficient(0.5f)
										->AddTab(UnLive2DAnimationBlueprintEditorTabs::CurveNamesTab, ETabState::ClosedTab)
										->AddTab(UnLive2DAnimationBlueprintEditorTabs::SkeletonTreeTab, ETabState::ClosedTab)
										->AddTab(FBlueprintEditorTabs::MyBlueprintID, ETabState::OpenedTab)
									}
								)
							}
						)
						->Split
						(
							// Middle 
							{
								FTabManager::NewSplitter()
								->SetOrientation(Orient_Vertical)
								->SetSizeCoefficient(0.55f)
								->Split
								(
									// Middle top - document edit area
									FTabManager::NewStack()
									->SetSizeCoefficient(0.8f)
									->AddTab("Document", ETabState::ClosedTab)
								)
								->Split
								(
									// Middle bottom - compiler results & find
									FTabManager::NewStack()
									->SetSizeCoefficient(0.2f)
									->AddTab(FBlueprintEditorTabs::CompilerResultsID, ETabState::ClosedTab)
									->AddTab(FBlueprintEditorTabs::FindResultsID, ETabState::ClosedTab)
								)
							}
						)
						->Split
						(
							// Right side
							{
								FTabManager::NewSplitter()
								->SetSizeCoefficient(0.2f)
								->SetOrientation(Orient_Vertical)
								->Split
								(
									// Right top - selection details panel & overrides
									FTabManager::NewStack()
									->SetHideTabWell(false)
									->SetSizeCoefficient(0.5f)
									->AddTab(FBlueprintEditorTabs::DetailsID, ETabState::OpenedTab)
									->AddTab(UnLive2DAnimationBlueprintEditorTabs::AdvancedPreviewTab, ETabState::OpenedTab)
									->AddTab(UnLive2DAnimationBlueprintEditorTabs::AssetOverridesTab, ETabState::ClosedTab)
									->SetForegroundTab(FBlueprintEditorTabs::DetailsID)
								)
								->Split
								(
									// Right bottom - Asset browser & advanced preview settings
									FTabManager::NewStack()
									->SetHideTabWell(false)
									->SetSizeCoefficient(0.5f)
									->AddTab(UnLive2DAnimationBlueprintEditorTabs::AnimBlueprintPreviewEditorTab, ETabState::OpenedTab)
									->AddTab(UnLive2DAnimationBlueprintEditorTabs::AssetBrowserTab, ETabState::OpenedTab)
									->AddTab(UnLive2DAnimationBlueprintEditorTabs::SlotNamesTab, ETabState::ClosedTab)
									->SetForegroundTab(UnLive2DAnimationBlueprintEditorTabs::AnimBlueprintPreviewEditorTab)
								)
							}
						)
					}
				)
			}
		);

	FUnLive2DManagerModule& MangerModule = FModuleManager::LoadModuleChecked<FUnLive2DManagerModule>("UnLive2DManager");
	// setup toolbar - clear existing toolbar extender from the BP mode
	//@TODO: Keep this in sync with BlueprintEditorModes.cpp
	ToolbarExtender = MakeShareable(new FExtender);
	if (UToolMenu* Toolbar = InAnimationBlueprintEditor->RegisterModeToolbarIfUnregistered(GetModeName()))
	{
		InAnimationBlueprintEditor->GetToolbarBuilder()->AddCompileToolbar(Toolbar);
		InAnimationBlueprintEditor->GetToolbarBuilder()->AddScriptingToolbar(Toolbar);
		InAnimationBlueprintEditor->GetToolbarBuilder()->AddBlueprintGlobalOptionsToolbar(Toolbar);
		InAnimationBlueprintEditor->GetToolbarBuilder()->AddDebuggingToolbar(Toolbar);
	}
	MangerModule.OnRegisterTabs().Broadcast(TabFactories, InAnimationBlueprintEditor);
	LayoutExtender = MakeShared<FLayoutExtender>();
	MangerModule.OnRegisterLayoutExtensions().Broadcast(*LayoutExtender.Get());
	TabLayout->ProcessExtensions(*LayoutExtender.Get());
}

void FUnLive2DAnimBlueprintEditorMode::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	TSharedPtr<FBlueprintEditor> BP = MyBlueprintEditor.Pin();

	BP->RegisterToolbarTab(InTabManager.ToSharedRef());

	// Mode-specific setup
	BP->PushTabFactories(CoreTabFactories);
	BP->PushTabFactories(BlueprintEditorTabFactories);
	BP->PushTabFactories(TabFactories);
}

void FUnLive2DAnimBlueprintEditorMode::PostActivateMode()
{
	if (UUnLive2DAnimBlueprint* AnimBlueprint = AnimBlueprintPtr.Get())
	{
	}

	FBlueprintEditorApplicationMode::PostActivateMode();
}

