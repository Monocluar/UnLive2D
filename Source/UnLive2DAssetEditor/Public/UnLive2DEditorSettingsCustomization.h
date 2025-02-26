#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IDetailLayoutBuilder;
class SGameplayTagWidget;

//////////////////////////////////////////////////////////////////////////
// FUnLive2DEditorSettingsCustomization

class UNLIVE2DASSETEDITOR_API FUnLive2DEditorSettingsCustomization : public IDetailCustomization
{
public:

	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
	// End of IDetailCustomization interface

	//void TextCommited(const FText& InText, ETextCommit::Type InCommitType, UDialogue* Dialogue, int32 id);
};