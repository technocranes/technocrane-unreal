// Copyright (c) 2020 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// TechnocranePlugin.cpp
// Sergei <Neill3d> Solokhin

#include "TechnocranePrivatePCH.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ITechnocranePlugin.h"
#include "technocrane_hardware.h"

#include "Interfaces/IPluginManager.h"

class FTechnocranePlugin : public ITechnocranePlugin
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

	/** Handle to the delay-loaded library. */
	void* TechnocraneLibHandle;

};

IMPLEMENT_MODULE(FTechnocranePlugin, TechnocranePlugin )

void TechnocraneLogCallback(const char* text, const int level)
{
	FString str_text(text);
	switch (level)
	{
	case 0x02:
		UE_LOG(LogTechnocrane, Warning, TEXT("%s"), *str_text);
		break;
	case 0x03:
		UE_LOG(LogTechnocrane, Error, TEXT("%s"), *str_text);
		break;
	default:
		UE_LOG(LogTechnocrane, Log, TEXT("%s"), *str_text);
	}
}

void FTechnocranePlugin::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
	check(TechnocraneLibHandle == nullptr);

	// Note: These paths correspond to the RuntimeDependency specified in the .Build.cs script.
	const FString PluginBaseDir = IPluginManager::Get().FindPlugin("TechnocranePlugin")->GetBaseDir();
	const FString TechnocraneDll = TEXT("TechnocraneLib.dll");

#if PLATFORM_WINDOWS && PLATFORM_64BITS
	FString LibraryPath = FPaths::Combine(*PluginBaseDir, TEXT("/Source/ThirdParty/TechnocraneSDK/lib/Win64"));
#elif PLATFORM_WINDOWS && PLATFORM_32BITS
	FString LibraryPath = FPaths::Combine(*PluginBaseDir, TEXT("/Source/ThirdParty/TechnocraneSDK/lib/Win32"));
#else
#	error Path to Technocrane shared library not specified for this platform!
#endif
	FPlatformProcess::PushDllDirectory(*LibraryPath);
	LibraryPath = FPaths::Combine(LibraryPath, TechnocraneDll);

	if (!FPaths::FileExists(LibraryPath))
    {
        UE_LOG(LogTechnocrane, Error, TEXT("Failed to find the binary folder for the dll. Plug-in will not be functional."));
        return;
    }

	TechnocraneLibHandle = FPlatformProcess::GetDllHandle(*LibraryPath);

	if (TechnocraneLibHandle == nullptr)
	{
		printf("error loading a library\n");		
		UE_LOG(LogTechnocrane, Error, TEXT("Failed to load required library %s. Plug-in will not be functional."), *TechnocraneDll);
        return;
	}

#if defined(TECHNOCRANESDK)
	NTechnocrane::SetLogCallback(TechnocraneLogCallback);
#endif
}


void FTechnocranePlugin::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// Unload the DLL.
	if (nullptr != TechnocraneLibHandle)
	{
		FPlatformProcess::FreeDllHandle(TechnocraneLibHandle);
		TechnocraneLibHandle = nullptr;
	}
}


DEFINE_LOG_CATEGORY(LogTechnocrane);

