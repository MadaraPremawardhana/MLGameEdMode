// Copyright. All rights reserved.
#include "MLGameRuntime.h"

DEFINE_LOG_CATEGORY(LogMLGame);

void FMLGameRuntimeModule::StartupModule()
{
	UE_LOG(LogMLGame, Log, TEXT("MLGameRuntime module started."));
}

void FMLGameRuntimeModule::ShutdownModule()
{
	UE_LOG(LogMLGame, Log, TEXT("MLGameRuntime module shut down."));
}

IMPLEMENT_MODULE(FMLGameRuntimeModule, MLGameRuntime)
