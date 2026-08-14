// Copyright. All rights reserved.
#pragma once

#include "Modules/ModuleManager.h"
#include "Logging/LogMacros.h"

MLGAMERUNTIME_API DECLARE_LOG_CATEGORY_EXTERN(LogMLGame, Log, All);

/**
 * Runtime module for the ML Game EdMode plugin.
 * Holds the ML model asset type, the CPU inference wrapper (built on Unreal's
 * Neural Network Engine / NNE), the reusable "ML Brain" actor component, and
 * the concrete example use cases (procedural animation, procedural generation).
 *
 * This module has no editor-only dependencies, so anything built on it keeps
 * working in packaged/shipping games - only the editor mode itself (in the
 * MLGameEdMode module) is editor-only.
 */
class FMLGameRuntimeModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
