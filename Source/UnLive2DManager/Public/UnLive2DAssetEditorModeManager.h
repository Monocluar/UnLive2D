
#pragma once

#include "CoreMinimal.h"
#include "AssetEditorModeManager.h"


class FUnLive2DAssetEditorModeManager : public FAssetEditorModeManager
{
public:
	/** 
	 * Get a camera target for when the user focuses the viewport
	 * @param OutTarget		The target object bounds
	 * @return true if the location is valid
	 */
	virtual bool GetCameraTarget(FSphere& OutTarget) const{ return false; };

	/** 
	 * Get debug info for any editor modes that are active
	 * @param	OutDebugText	The text to draw
	 */
	virtual void GetOnScreenDebugInfo(TArray<FText>& OutDebugText) const {};
};