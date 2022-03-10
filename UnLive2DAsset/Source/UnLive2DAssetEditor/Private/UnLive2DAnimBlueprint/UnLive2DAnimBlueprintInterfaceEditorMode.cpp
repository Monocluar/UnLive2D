#include "UnLive2DAnimBlueprintInterfaceEditorMode.h"
#include "BlueprintEditor.h"
#include "UnLive2DAnimationBlueprintEditor.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "UnLive2DManagerModule.h"
#include "BlueprintEditorTabs.h"

FUnLive2DAnimBlueprintInterfaceEditorMode::FUnLive2DAnimBlueprintInterfaceEditorMode(const TSharedRef<FUnLive2DAnimationBlueprintEditor>& InAnimationBlueprintEditor)
	: FBlueprintInterfaceApplicationMode(InAnimationBlueprintEditor, FUnLive2DAnimBlueprintEditorModes::AnimationBlueprintInterfaceEditorMode, FUnLive2DAnimBlueprintEditorModes::GetLocalizedMode)
{
	AnimBlueprintPtr = CastChecked<UUnLive2DAnimBlueprint>(InAnimationBlueprintEditor->GetBlueprintObj());

	TabLayout = FTabManager::NewLayout( "Standalone_UnLive2DAnimBlueprintInterfaceEditorMode_Layout_v1.0" )
		->AddArea
		(
			{
				FTabManager::NewPrimaryArea()
				->SetOrientation(Orient_Vertical)
				->Split
				(
					{
						// Top toolbar
						FTabManager::NewStack()
						->SetSizeCoefficient(0.186721f)
						->SetHideTabWell(true)
						->AddTab(InAnimationBlueprintEditor->GetToolbarTabId(), ETabState::OpenedTab)
					}
				)
				->Split
				(
					{
						// Main application area
						FTabManager::NewSplitter()
						->SetOrientation(Orient_Horizontal)
						->Split
						(
							{
								// Middle 
								FTabManager::NewSplitter()
								->SetOrientation(Orient_Vertical)
								->SetSizeCoefficient(0.75f)
								->Split
								(
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
							{
								// Right side
								FTabManager::NewSplitter()
								->SetSizeCoefficient(0.25f)
								->SetOrientation(Orient_Vertical)
								->Split
								(
									// Right top - details and my blueprint
									FTabManager::NewStack()
									->SetHideTabWell(false)
									->SetSizeCoefficient(0.5f)
									->AddTab(FBlueprintEditorTabs::MyBlueprintID, ETabState::OpenedTab)
								)
								->Split
								(
									// Middle bottom - compiler results & find
									FTabManager::NewStack()
									->SetSizeCoefficient(0.5f)
									->AddTab(FBlueprintEditorTabs::DetailsID, ETabState::OpenedTab)
								)
							}
						)
					}
				)
			}
		);

	FUnLive2DManagerModule& MangerModule = FModuleManager::LoadModuleChecked<FUnLive2DManagerModule>("UnLive2DManager");

	ToolbarExtender = MakeShareable(new FExtender);

	if (UToolMenu* Toolbar = InAnimationBlueprintEditor->RegisterModeToolbarIfUnregistered(GetModeName()))
	{
		InAnimationBlueprintEditor->GetToolbarBuilder()->AddCompileToolbar(Toolbar);
		InAnimationBlueprintEditor->GetToolbarBuilder()->AddBlueprintGlobalOptionsToolbar(Toolbar);
	}

	MangerModule.OnRegisterTabs().Broadcast(TabFactories, InAnimationBlueprintEditor);
	LayoutExtender = MakeShared<FLayoutExtender>();
	MangerModule.OnRegisterLayoutExtensions().Broadcast(*LayoutExtender.Get());
	TabLayout->ProcessExtensions(*LayoutExtender.Get());
}

void FUnLive2DAnimBlueprintInterfaceEditorMode::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	TSharedPtr<FBlueprintEditor> BP = MyBlueprintEditor.Pin();

	BP->RegisterToolbarTab(InTabManager.ToSharedRef());

	// Mode-specific setup
	BP->PushTabFactories(BlueprintInterfaceTabFactories);
}

