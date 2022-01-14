#pragma once
#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

class FUnLive2DEditorStyle
{
public:
	static void Initialize();
	static void Shutdown();
	static TSharedPtr< class ISlateStyle > Get();
	static FName GetStyleSetName();
private:
	static TSharedPtr< class FSlateStyleSet > StyleSet;
};