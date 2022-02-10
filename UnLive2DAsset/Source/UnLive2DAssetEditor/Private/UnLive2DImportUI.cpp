#include "UnLive2DImportUI.h"

UUnLive2DImportUI::UUnLive2DImportUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsImportTexture = true;

	this->SetFlags(RF_Transactional);
}

void UUnLive2DImportUI::ResetToDefault()
{
	ReloadConfig();
}

