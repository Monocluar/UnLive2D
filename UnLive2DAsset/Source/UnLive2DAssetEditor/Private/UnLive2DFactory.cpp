
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
#include "UnLive2DAssetEditor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Animation/UnLive2DMotion.h"
#include "Animation/UnLive2DExpression.h"

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
	// ??????????????????
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

		if (UnLive2DOptionWindow->ShouldImport()) //????????????
		{
			UnLive2DPtr = NewObject<UUnLive2D>(InParent, InClass, InName, Flags | RF_Transactional);

			TArray<FString> TexturePaths;
			TArray<FUnLive2DMotionData> LoadMotionDataArr;
			TMap<FString, FUnLiveByteData> LoadExpressionArr;
			UnLive2DPtr->LoadLive2DFileDataFormPath(Live2DDataPath, TexturePaths, LoadMotionDataArr, LoadExpressionArr);

			if (ImportUI->bIsImportTexture) // ??????????????????
			{
				FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");

				const FString TargetTexturePath = FPackageName::GetLongPackagePath(InParent->GetOutermost()->GetPathName()) / InName.ToString() / TEXT("Textures");

				TArray<UObject*> ImportedAssets = AssetToolsModule.Get().ImportAssets(TexturePaths, TargetTexturePath);

				for (UObject*& Asset : ImportedAssets)
				{
					if (UTexture2D* TextureAsset = Cast<UTexture2D>(Asset))
					{
						UnLive2DPtr->TextureAssets.Add(TextureAsset);
					}
				}
			}

			if (ImportUI->bIsImportMotion) // ??????????????????
			{
				
				for (FUnLive2DMotionData& Item : LoadMotionDataArr)
				{
					UUnLive2DMotion* Motion = CreateAsset<UUnLive2DMotion>(InParent->GetOutermost()->GetPathName(), InName.ToString() / TEXT("Motions"), Item.GetFPathName());
					Motion->SetLive2DMotionData(Item);
					Motion->UnLive2D = UnLive2DPtr;
				}
			}

			if (ImportUI->bIsImportExpression) //??????????????????
			{
				for (auto& Item : LoadExpressionArr)
				{
					UUnLive2DExpression* Expression = CreateAsset<UUnLive2DExpression>(InParent->GetOutermost()->GetPathName(), InName.ToString() / TEXT("Expression"), Item.Key);
					Expression->SetLive2DExpressionData(Item.Value);
					Expression->UnLive2D = UnLive2DPtr;
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

UObject* UUnLive2DFactory::CreateAssetOfClass(UClass* AssetClass, FString ParentPackageName, FString ItemizeName,  FString ObjectName, bool bAllowReplace /*= false*/)
{
	// ???????????????????????????????????????
	//UObject* ParentPkg = CreatePackage(*ParentPackageName);
	FString ParentPath = FString::Printf(TEXT("%s/%s/%s"), *FPackageName::GetLongPackagePath(*ParentPackageName), *ItemizeName, *ObjectName);
	UObject* Parent = CreatePackage( *ParentPath);

	// ??????????????????????????????????????????
	UObject* Object = LoadObject<UObject>(Parent, *ObjectName, NULL, LOAD_NoWarn | LOAD_Quiet, NULL);

	// ?????????????????????????????????????????????????????????
	if ((Object != nullptr) && (Object->GetClass() != AssetClass))
	{
		UE_LOG(LogUnLive2DEditor, Error, TEXT("UnLive2DFactory : ?????????????????????????????????????????????"));
		return nullptr;
	}

	if (Object != nullptr && ! bAllowReplace)
	{
		UE_LOG(LogUnLive2DEditor, Warning, TEXT("UnLive2DFactory : ????????????%s ???????????????"), *ParentPath);
	}

	if (Object == nullptr)
	{
		// ???????????????????????????????????????????????????
		Object = NewObject<UObject>(Parent, AssetClass, *ObjectName, RF_Public | RF_Standalone);
		Object->MarkPackageDirty();
		// ?????????????????????
		FAssetRegistryModule::AssetCreated(Object);
	}

	return Object;
}

#undef LOCTEXT_NAMESPACE