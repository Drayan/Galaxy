#include "GalaxyShadersModule.h"

IMPLEMENT_MODULE(FGalaxyShadersModule, GalaxyShaders)

void FGalaxyShadersModule::StartupModule()
{
	FString baseDir = FPaths::Combine(FPaths::GameSourceDir(), TEXT("GalaxyShaders"));
	FString shadersDir = FPaths::Combine(baseDir, TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/GalaxyShaders"), shadersDir);
}
