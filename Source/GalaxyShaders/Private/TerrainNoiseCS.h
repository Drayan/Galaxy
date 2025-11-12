// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0. Noncommercial use only. Commercial use requires written permission. See https://polyformproject.org/licenses/noncommercial/1.0.0/
#pragma once

#include "GlobalShader.h"
#include "ShaderParameterStruct.h"

class FTerrainNoiseCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FTerrainNoiseCS);
	SHADER_USE_PARAMETER_STRUCT(FTerrainNoiseCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(float, Frequency)
		SHADER_PARAMETER(float, Amplitude)
		SHADER_PARAMETER(float, Lacunarity)
		SHADER_PARAMETER(float, Persistence)
		SHADER_PARAMETER(int, Octaves)
		SHADER_PARAMETER(float, Radius)
		SHADER_PARAMETER(FVector2f, PatchMin)
		SHADER_PARAMETER(FVector2f, PatchMax)
		SHADER_PARAMETER(FVector3f, PlanetCenter)
		SHADER_PARAMETER(FVector3f, FaceX)
		SHADER_PARAMETER(FVector3f, FaceY)
		SHADER_PARAMETER(FVector3f, FaceZ)
		SHADER_PARAMETER(uint32, Seed)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, OutHeight)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<FVector3f>, OutNormal)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<FColor>, OutColor)
	END_SHADER_PARAMETER_STRUCT();

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM6);
	}
};