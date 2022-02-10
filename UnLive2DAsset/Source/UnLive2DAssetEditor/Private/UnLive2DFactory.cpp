
#include "UnLive2DFactory.h"
#include "UnLive2D.h"
#include "AssetImportTask.h"
#include "Misc/FeedbackContext.h"
#include "Framework/Application/SlateApplication.h"
#include "UnLive2DImportOptionWindow/UnLive2DOptionWindow.h"
#include "UnLive2DImportUI.h"
#include "Misc/AutomationTest.h"
#include "Interfaces/IMainFrameModule.h"
#include "HAL/PlatformApplicationMisc.h"
#include "AssetToolsModule.h"

#define LOCTEXT_NAMESPACE "FUnLive2DAssetEditorModule"

UUnLive2DFactory::UUnLive2DFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    SupportedClass = UUnLive2D::StaticClass();

	Formats.Add(TEXT("moc3;Live2D Format"));
	Formats.Add(TEXT("json;Live2D Format(Model3.json)"));

	//bCreateNew = true;
	bEditAfterNew = true;
	bEditorImport = true;
	bText         = false;

	ImportPriority = DefaultImportPriority + 1;
}

bool UUnLive2DFactory::DoesSupportClass(UClass* Class)
{
    return UUnLive2D::StaticClass() == Class;
}
/*

UObject* UUnLive2DFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UUnLive2D>(InParent, Class, Name, Flags | RF_Transactional);
}*/

bool UUnLive2DFactory::FactoryCanImport(const FString& Filename)
{
	const FString Extension = FPaths::GetExtension(Filename);

	if (Extension == TEXT("moc3"))
	{
		return true;
	}
	else if (Extension == TEXT("json"))
	{
		return Filename.Contains(TEXT(".model3.json"));
	}

	return false;
}

UObject* UUnLive2DFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	const FString Extension = FPaths::GetExtension(Filename);

	FString Live2DDataPath = Filename;
	if (Extension == TEXT("moc3"))
	{
		Live2DDataPath = Filename.LeftChop(Extension.Len()) + TEXT("model3.json");
	}
	else if (!Filename.Contains(TEXT(".model3.json")))
	{
		return nullptr;
	}

	UUnLive2D* UnLive2DPtr = nullptr;
	// 导入设置窗体
	{
		TSharedPtr<SWindow> ParentWindow;
		if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
		{
			IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
			ParentWindow = MainFrame.GetParentWindow();
		}

		const float UnLive2DImportWindowWidth = 410.f;
		const float UnLive2DImportWindowHeight = 560.f;
		FVector2D UnLive2DImportWindowSize = FVector2D(UnLive2DImportWindowWidth, UnLive2DImportWindowHeight);

		FSlateRect WorkAreaRect = FSlateApplicationBase::Get().GetPreferredWorkArea();
		FVector2D DisplayTopLeft(WorkAreaRect.Left, WorkAreaRect.Top);
		FVector2D DisplaySize(WorkAreaRect.Right - WorkAreaRect.Left, WorkAreaRect.Bottom - WorkAreaRect.Top);

		float ScaleFactor = FPlatformApplicationMisc::GetDPIScaleFactorAtPoint(DisplayTopLeft.X, DisplayTopLeft.Y);
		UnLive2DImportWindowSize *= ScaleFactor;

		FVector2D WindowPosition = (DisplayTopLeft + (DisplaySize - UnLive2DImportWindowSize) / 2.0f) / ScaleFactor;

		TSharedRef<SWindow> Window = SNew(SWindow)
			.Title(NSLOCTEXT("UnLive2DEd", "UnLive2DImportOpionsTitle", "UnLive2D Import Options"))
			.SizingRule(ESizingRule::Autosized)
			.AutoCenter(EAutoCenter::None)
			.ClientSize(UnLive2DImportWindowSize)
			.ScreenPosition(WindowPosition);

		TSharedPtr<SUnLive2DOptionWindow> UnLive2DOptionWindow;
		Window->SetContent
		(
			SAssignNew(UnLive2DOptionWindow, SUnLive2DOptionWindow)
			.ImportUI(ImportUI)
			.WidgetWindow(Window)
			.FullPath(FName(InParent->GetPathName()))
			.MaxWindowHeight(UnLive2DImportWindowHeight)
			.MaxWindowWidth(UnLive2DImportWindowWidth)
		);


		FSlateApplication::Get().AddModalWindow(Window, ParentWindow, false);

		if (UnLive2DOptionWindow->ShouldImport()) //是否导入
		{
			UnLive2DPtr = NewObject<UUnLive2D>(InParent, InClass, InName, Flags | RF_Transactional);

			TArray<FString> TexturePaths;

			UnLive2DPtr->LoadLive2DFileDataFormPath(Live2DDataPath, TexturePaths);

			if (ImportUI->bIsImportTexture) // 是否导入图片
			{
				FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");

				const FString TargetTexturePath = FPackageName::GetLongPackagePath(InParent->GetOutermost()->GetPathName()) / TEXT("Textures");

				TArray<UObject*> ImportedAssets = AssetToolsModule.Get().ImportAssets(TexturePaths, TargetTexturePath);

				for (UObject*& Asset : ImportedAssets)
				{
					if (UTexture2D* TextureAsset = Cast<UTexture2D>(Asset))
					{
						UnLive2DPtr->TextureAssets.Add(TextureAsset);
					}
				}
			}
		}

	}

	return UnLive2DPtr;
}

void UUnLive2DFactory::PostInitProperties()
{
	Super::PostInitProperties();
	ImportUI = NewObject<UUnLive2DImportUI>(this, NAME_None, RF_NoFlags);
}

#undef LOCTEXT_NAMESPACE