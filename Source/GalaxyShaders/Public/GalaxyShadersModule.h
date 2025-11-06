#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class FGalaxyShadersModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
};
