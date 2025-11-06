#include "TerrainNoiseDispatcher.h"
#include "RenderGraphBuilder.h"
#include "RenderTargetPool.h"
#include "RHICommandList.h"
#include "RenderGraphUtils.h"

void FTerrainNoiseDispatcher::DispatchToRenderTarget(UTextureRenderTarget2D* RT, const FTerrainNoiseParams& P)
{
	if (!RT) return;
	FTextureRenderTargetResource* RTRes = RT->GameThread_GetRenderTargetResource();
	const FIntPoint Size = RTRes->GetSizeXY();

	ENQUEUE_RENDER_COMMAND(TerrainNoiseDispatch)(
		[RTRes, Size, P](FRHICommandListImmediate& RHICmdList)
		{
			FRDGBuilder GraphBuilder(RHICmdList, RDG_EVENT_NAME("SphericalTerrain_Noise"));

			// Save the render target to the graph as external texture
			FRDGTextureRef RDGOutTex = GraphBuilder.RegisterExternalTexture(
				CreateRenderTarget(RTRes->GetRenderTargetTexture(), TEXT("TerrainHeigthRT"))
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

			TShaderMapRef<FTerrainNoiseCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

			const FIntVector GroupCount(
				FMath::DivideAndRoundUp(Size.X, 8),
				FMath::DivideAndRoundUp(Size.Y, 8),
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
