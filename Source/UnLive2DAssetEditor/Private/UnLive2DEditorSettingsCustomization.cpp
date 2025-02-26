#include "UnLive2DEditorSettingsCustomization.h"

#define LOCTEXT_NAMESPACE "FUnLive2DAssetEditorModule"

TSharedRef<IDetailCustomization> FUnLive2DEditorSettingsCustomization::MakeInstance()
{
	return MakeShareable(new FUnLive2DEditorSettingsCustomization());
}

void FUnLive2DEditorSettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{

}

#undef LOCTEXT_NAMESPACE