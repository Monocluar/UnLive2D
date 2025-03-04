#include "UnLive2DImportUI.h"

UUnLive2DImportUI::UUnLive2DImportUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsImportTexture = true;
	bIsImportMotion = true;
	bIsImportExpression = true;
	bIsImportPhysics = true;
	bIsImportDisplayInfo = true;
	this->SetFlags(RF_Transactional);
}

void UUnLive2DImportUI::ResetToDefault()
{
	ReloadConfig();
}

