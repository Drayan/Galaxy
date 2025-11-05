// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0.
// Noncommercial use only. Commercial use requires written permission.
// See https://polyformproject.org/licenses/noncommercial/1.0.0/
#include "SphericalTerrain.h"
#if WITH_EDITOR
#include "LevelEditorViewport.h"
#endif

FPlayerViewInfo GetCurrentViewInfo(UWorld* World)
{
	FPlayerViewInfo viewInfo = {};
	if (!World) return viewInfo;

	// Game
	if (APlayerController* PC = World->GetFirstPlayerController())
		if (APlayerCameraManager* camera = PC->PlayerCameraManager)
		{
			viewInfo.CameraPosition = camera->GetCameraLocation();
			viewInfo.FOVY_Rad = FMath::DegreesToRadians(camera->GetFOVAngle());
			int32 sizeX, sizeY;
			PC->GetViewportSize(sizeX, sizeY);
			viewInfo.ScreenHeight = FMath::Max(sizeY, 1);

			return viewInfo;
		}

#if WITH_EDITOR
	// Editor
	if (GEditor && GCurrentLevelEditingViewportClient)
	{
		const auto* viewportClient = GCurrentLevelEditingViewportClient;
		viewInfo.CameraPosition = viewportClient->GetViewLocation();
		viewInfo.FOVY_Rad = FMath::DegreesToRadians(viewportClient->ViewFOV);
		const FIntPoint size = viewportClient->Viewport->GetSizeXY();
		viewInfo.ScreenHeight = FMath::Max(size.Y, 1);
	}
#endif

	return viewInfo;
}

// Sets default values
ASphericalTerrain::ASphericalTerrain()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	TerrainFaces.SetNum(6);
	for(int32 i = 0; i < 6; ++i)
	{
		TerrainFaces[i] = CreateDefaultSubobject<USphericalTerrainFace>(*FString::Printf(TEXT("TerrainFace%d"), i));
		TerrainFaces[i]->SetupAttachment(RootComponent);
		TerrainFaces[i]->SetMobility(EComponentMobility::Static);
	}
}

void ASphericalTerrain::OnConstruction(const FTransform& Transform)
{
	SetupFaces();
}

void ASphericalTerrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FPlayerViewInfo viewInfo = GetCurrentViewInfo(GetWorld());
	for (USphericalTerrainFace* face : TerrainFaces)
	{
		//if (!face) continue;
		face->ComputeLOD(viewInfo);
	}
}

void ASphericalTerrain::SetupFaces()
{
	for (int32 i = 0; i < 6; ++i)
	{
		USphericalTerrainFace* face = TerrainFaces.IsValidIndex(i) ? TerrainFaces[i] : nullptr;
		if (face)
		{
			face->ParentTerrain = this;
			face->GridSize = MeshResolution;
			face->FaceDirection = static_cast<ETerrainFaceDirection::Type>(i);
			switch (face->FaceDirection)
			{
				case ETerrainFaceDirection::Up:
				{
					FBox limitsUp(FVector(-Radius, -Radius, Radius), FVector(Radius, Radius, Radius));
					face->Limits = limitsUp;
				} break;

				case ETerrainFaceDirection::Down:
				{
					FBox limitsDown(FVector(-Radius, -Radius, -Radius), FVector(Radius, Radius, -Radius));
					face->Limits = limitsDown;
				} break;

				case ETerrainFaceDirection::Left:
				{
					FBox limitsLeft(FVector(Radius, -Radius, -Radius), FVector(Radius, Radius, Radius));
					face->Limits = limitsLeft;
				} break;

				case ETerrainFaceDirection::Right:
				{
					FBox limitsRight(FVector(-Radius, -Radius, -Radius), FVector(-Radius, Radius, Radius));
					face->Limits = limitsRight;
				} break;

				case ETerrainFaceDirection::Forward:
				{
					FBox limitsForward(FVector(-Radius, Radius, -Radius), FVector(Radius, Radius, Radius));
					face->Limits = limitsForward;
				} break;

				case ETerrainFaceDirection::Backward:
				{
					FBox limitsBackward(FVector(-Radius, -Radius, -Radius), FVector(Radius, -Radius, Radius));
					face->Limits = limitsBackward;
				} break;
			}
			face->OnBuild();
		}
	}
}

#if WITH_EDITOR
void ASphericalTerrain::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	SetupFaces();
}
#endif
