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
				CreateRenderTarget(HeightRTRes->GetRenderTargetTexture(), TEXT("TerrainHeigthRT"))
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
