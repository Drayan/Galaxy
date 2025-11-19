// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0. 
// Noncommercial use only. Commercial use requires written permission. 
// See https://polyformproject.org/licenses/noncommercial/1.0.0/
#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2DArray.h"

struct FTerrainNoiseParams
{
	float Frequency = 1.5f;
	float Amplitude = 100.0f;
	float Lacunarity = 2.0f;
	float Persistence = 0.5f;
	int32 Octaves = 6;

	float Radius = 500.0f;

	FVector2f PatchMin = FVector2f::ZeroVector;
	FVector2f PatchMax = FVector2f::One();

	FVector3f PlanetCenter = FVector3f::ZeroVector;
	FVector3f FaceX = FVector3f::ForwardVector;
	FVector3f FaceY = FVector3f::RightVector;
	FVector3f FaceZ = FVector3f::UpVector;

	uint32 Seed = 1337;
};

class FTerrainNoiseDispatcher
{
public:
	static GALAXYSHADERS_API void DispatchToRenderTarget(
		UTextureRenderTarget2D* HeightRT,
		UTextureRenderTarget2D* ColorRT,
		UTextureRenderTarget2D* NormalRT,
		const FTerrainNoiseParams& P);

	static GALAXYSHADERS_API void DispatchToTextureArraySlice(
		UTexture2DArray* HeightArray,
		UTexture2DArray* ColorArray,
		UTexture2DArray* NormalArray,
		int32 SliceIndex,
		const FTerrainNoiseParams& P);
};
