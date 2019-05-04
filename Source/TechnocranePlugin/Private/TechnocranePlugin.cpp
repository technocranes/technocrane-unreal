// Copyright (c) 2019 Technocrane s.r.o. 
// 
// TechnocranePlugin.cpp
// Sergei <Neill3d> Solokhin 2019s

#include "TechnocranePrivatePCH.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ITechnocranePlugin.h"

#include "Interfaces/IPluginManager.h"
//#include <Paths.h>
//#include <PlatformProcess.h>

//DEFINE_LOG_CATEGORY(LogTechnocrane);

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



void FTechnocranePlugin::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)

	// Note: These paths correspond to the RuntimeDependency specified in the .Build.cs script.
	const FString PluginBaseDir = IPluginManager::Get().FindPlugin("TechnocranePlugin")->GetBaseDir();

#if PLATFORM_WINDOWS && PLATFORM_64BITS
	const FString LibraryPath = FPaths::Combine(*PluginBaseDir, TEXT("ThirdParty/TechnocraneSDK/lib/Win64/TechnocraneLib.dll"));
#elif PLATFORM_WINDOWS && PLATFORM_32BITS
	const FString LibraryPath = FPaths::Combine(*PluginBaseDir, TEXT("ThirdParty/TechnocraneSDK/lib/Win32/TechnocraneLib.dll"));
#else
#	error Path to Technocrane shared library not specified for this platform!
#endif

	TechnocraneLibHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

	if (nullptr == TechnocraneLibHandle)
	{
		printf("error loading a library\n");		
		//FMessageDialog::Open(EAppMsgType::Ok, TCHAR_TO_);
	}
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



