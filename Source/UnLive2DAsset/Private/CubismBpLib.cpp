// Fill out your copyright notice in the Description page of Project Settings.

#include "CubismBpLib.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

#include "FWPort/CubismFrameWorkAllocator.h"
#include "Interfaces/IPluginManager.h"
#include "Templates/SharedPointer.h"

#include "UnLive2DAssetModule.h"

using namespace Live2D::Cubism::Core;
using namespace Live2D::Cubism::Framework;

CubismFrameWorkAllocator Allocator;

/**	<把UE4 Log信息注册到Live2D中 */
static void Cubism_Log(const char* Message)
{
	UE_LOG(LogLive2D, Log, TEXT("[UCubismBpLib] %s"), ANSI_TO_TCHAR(Message));
}

/////////////////////////////////////////////////////////////////////////////////////
static bool bIsInited = false;

void UCubismBpLib::InitCubism()
{
	if (bIsInited)
	{
		UE_LOG(LogUnLive2D, Log, TEXT("UCubismBpLib::InitCubism: Already inited."));
		return;
	}

	csmVersion Version = csmGetVersion();
	UE_LOG(LogLive2D, Log, TEXT("csm Version is: %u"), Version);

	Csm::CubismFramework::Option _cubismOption;  ///< Cubism SDK Option
        //setup cubism
    _cubismOption.LogFunction = Cubism_Log;
    _cubismOption.LoggingLevel = Live2D::Cubism::Framework::CubismFramework::Option::LogLevel_Verbose;
    Csm::CubismFramework::StartUp(&Allocator, &_cubismOption);

    //Initialize cubism
    CubismFramework::Initialize();

	bIsInited = true;
}

FString UCubismBpLib::GetCubismPath()
{
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("UnLive2DAsset"));
	if (Plugin.IsValid())
	{
        return Plugin->GetBaseDir();
	}
	else
	{
		return FPaths::ProjectPluginsDir() / TEXT("UnLive2DAsset");
	}
}
