// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0. 
// Noncommercial use only. Commercial use requires written permission. 
// See https://polyformproject.org/licenses/noncommercial/1.0.0/
#include "TerrainNoiseDispatcher.h"
#include "RenderGraphBuilder.h"
#include "RenderTargetPool.h"
#include "RHICommandList.h"
#include "RenderGraphUtils.h"
#include "TerrainNoiseCS.h"

void FTerrainNoiseDispatcher::DispatchToRenderTarget(
	UTextureRenderTarget2D* HeightRT,
	UTextureRenderTarget2D* ColorRT,
	UTextureRenderTarget2D* NormalRT,
	const FTerrainNoiseParams& P)
{
	if (!HeightRT || !ColorRT || !NormalRT) return;

	FTextureRenderTargetResource* HeightRTRes = HeightRT->GameThread_GetRenderTargetResource();
	FTextureRenderTargetResource* ColorRTRes = ColorRT->GameThread_GetRenderTargetResource();
	FTextureRenderTargetResource* NormalRTRes = NormalRT->GameThread_GetRenderTargetResource();
	const FIntPoint HeightMapSize = HeightRTRes->GetSizeXY();

	ENQUEUE_RENDER_COMMAND(TerrainNoiseDispatch)(
		[HeightRTRes, HeightMapSize, ColorRTRes, NormalRTRes, P](FRHICommandListImmediate& RHICmdList)
		{
			FRDGBuilder GraphBuilder(RHICmdList, RDG_EVENT_NAME("SphericalTerrain_Noise"));

			// Save the render target to the graph as external texture
			FRDGTextureRef RDGOutTex = GraphBuilder.RegisterExternalTexture(
				CreateRenderTarget(HeightRTRes->GetRenderTargetTexture(), TEXT("TerrainHeightRT"))
			);

			FRDGTextureRef RDGOutColor = GraphBuilder.RegisterExternalTexture(
				CreateRenderTarget(ColorRTRes->GetRenderTargetTexture(), TEXT("TerrainColorRT"))
			);

			FRDGTextureRef RDGOutNormal = GraphBuilder.RegisterExternalTexture(
				CreateRenderTarget(NormalRTRes->GetRenderTargetTexture(), TEXT("TerrainNormalRT"))
			);

			FTerrainNoiseCS::FParameters* Params = GraphBuilder.AllocParameters<FTerrainNoiseCS::FParameters>();
			Params->Frequency = P.Frequency;
			Params->Amplitude = P.Amplitude;
			Params->Lacunarity = P.Lacunarity;
			Params->Persistence = P.Persistence;
			Params->Octaves = P.Octaves;
			Params->Radius = P.Radius;
			Params->PatchMin = P.PatchMin;
			Params->PatchMax = P.PatchMax;
			Params->PlanetCenter = P.PlanetCenter;
			Params->FaceX = P.FaceX;
			Params->FaceY = P.FaceY;
			Params->FaceZ = P.FaceZ;
			Params->Seed = P.Seed;
			Params->UseTextureArray = 0;
			Params->SliceIndex = 0;
			Params->OutHeight = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(RDGOutTex));
			Params->OutNormal = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(RDGOutNormal));
			Params->OutColor = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(RDGOutColor));

			TShaderMapRef<FTerrainNoiseCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

			const FIntVector GroupCount(
				FMath::DivideAndRoundUp(HeightMapSize.X, 8),
				FMath::DivideAndRoundUp(HeightMapSize.Y, 8),
				1
			);

			FComputeShaderUtils::AddPass(
				GraphBuilder,
				RDG_EVENT_NAME("TerrainNoiseCS"),
				ComputeShader,
				Params,
				GroupCount
			);

			GraphBuilder.Execute();
		});
}

void FTerrainNoiseDispatcher::DispatchToTextureArraySlice(
	UTexture2DArray* HeightArray,
	UTexture2DArray* ColorArray,
	UTexture2DArray* NormalArray,
	int32 SliceIndex,
	const FTerrainNoiseParams& P)
{
	if (!HeightArray || !ColorArray || !NormalArray)
	{
		UE_LOG(LogTemp, Error, TEXT("Nullptr texture array passed to DispatchToTextureArraySlice"));
		return;
	}

	const FIntPoint TextureSize(HeightArray->GetSizeX(), HeightArray->GetSizeY());

	ENQUEUE_RENDER_COMMAND(TerrainNoiseArrayDispatch)(
		[HeightArray, ColorArray, NormalArray, SliceIndex, TextureSize, P](FRHICommandListImmediate& RHICmdList)
		{
			FRDGBuilder GraphBuilder(RHICmdList, RDG_EVENT_NAME("TerrainNoise_Array_Slice%d", SliceIndex));

			FRHITexture* HeightRHI = HeightArray->GetResource()->TextureRHI;
			FRHITexture* ColorRHI = ColorArray->GetResource()->TextureRHI;
			FRHITexture* NormalRHI = NormalArray->GetResource()->TextureRHI;

			FRDGTextureRef HeightTex = GraphBuilder.RegisterExternalTexture(
				CreateRenderTarget(HeightRHI, TEXT("TerrainHeightArray")));
			FRDGTextureRef ColorTex = GraphBuilder.RegisterExternalTexture(
				CreateRenderTarget(ColorRHI, TEXT("TerrainColorArray")));
			FRDGTextureRef NormalTex = GraphBuilder.RegisterExternalTexture(
				CreateRenderTarget(NormalRHI, TEXT("TerrainNormalArray")));

			FTerrainNoiseCS::FParameters* Params = GraphBuilder.AllocParameters<FTerrainNoiseCS::FParameters>();
			Params->Frequency = P.Frequency;
			Params->Amplitude = P.Amplitude;
			Params->Lacunarity = P.Lacunarity;
			Params->Persistence = P.Persistence;
			Params->Octaves = P.Octaves;
			Params->Radius = P.Radius;
			Params->PatchMin = P.PatchMin;
			Params->PatchMax = P.PatchMax;
			Params->PlanetCenter = P.PlanetCenter;
			Params->FaceX = P.FaceX;
			Params->FaceY = P.FaceY;
			Params->FaceZ = P.FaceZ;
			Params->Seed = P.Seed;
			Params->UseTextureArray = 1;
			Params->SliceIndex = SliceIndex;
			Params->OutHeightArray = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(HeightTex, 0));
			Params->OutColorArray = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(ColorTex, 0));
			Params->OutNormalArray = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(NormalTex, 0));

			TShaderMapRef<FTerrainNoiseCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

			const FIntVector GroupCount(
				FMath::DivideAndRoundUp(TextureSize.X, 8),
				FMath::DivideAndRoundUp(TextureSize.Y, 8),
				1
			);

			FComputeShaderUtils::AddPass(
				GraphBuilder,
				RDG_EVENT_NAME("TerrainNoiseCS_Slice%d", SliceIndex),
				ComputeShader,
				Params,
				GroupCount
			);

			GraphBuilder.Execute();
		}
	);
}
