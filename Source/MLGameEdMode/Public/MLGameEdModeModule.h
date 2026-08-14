// Copyright. All rights reserved.
#pragma once

#include "Modules/ModuleManager.h"

/** Editor module for the ML Game EdMode plugin: registers/unregisters FMLGameEdMode with the level editor's mode toolbar. */
class FMLGameEdModeModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
