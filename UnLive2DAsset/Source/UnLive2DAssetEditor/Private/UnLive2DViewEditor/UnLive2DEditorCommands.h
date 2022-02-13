#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "UnLive2DEditorStyle.h"

class FUnLive2DEditorCommands : public TCommands<FUnLive2DEditorCommands>
{
public:
	FUnLive2DEditorCommands()
		: TCommands<FUnLive2DEditorCommands>(
			TEXT("UnLive2DEditor"), // Context name for fast lookup
			NSLOCTEXT("Contexts", "UnLive2DEditor", "UnLive2D Editor"), // Localized context name for displaying
			NAME_None, // Parent
			FUnLive2DEditorStyle::Get()->GetStyleSetName() // Icon Style Set
			)
	{
	}

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> EnterViewMode;
	TSharedPtr<FUICommandInfo> EnterAnimMode;
};