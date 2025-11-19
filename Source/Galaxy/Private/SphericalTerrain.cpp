// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0.
// Noncommercial use only. Commercial use requires written permission.
// See https://polyformproject.org/licenses/noncommercial/1.0.0/
#include "SphericalTerrain.h"
#include "SphericalTerrainFace.h"
#include "TerrainNoiseDispatcher.h"
#include "Engine/Texture2DArray.h"
#include "Materials/MaterialInstanceDynamic.h"

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

ASphericalTerrain::ASphericalTerrain()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

#if WITH_EDITOR
	PrimaryActorTick.bTickEvenWhenPaused = true;
#endif

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	TerrainFaces.SetNum(6);
	for (int32 i = 0; i < 6; ++i)
	{
		TerrainFaces[i] = CreateDefaultSubobject<USphericalTerrainFace>(*FString::Printf(TEXT("TerrainFace%d"), i));
		TerrainFaces[i]->SetupAttachment(RootComponent);
		TerrainFaces[i]->SetMobility(EComponentMobility::Static);
	}
}

void ASphericalTerrain::BeginPlay()
{
	Super::BeginPlay();

	if (!bTextureArraysInitialized)
	{
		InitializeTextureArrays();
	}
}

void ASphericalTerrain::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!bTextureArraysInitialized)
	{
		InitializeTextureArrays();
	}

	SetupFaces();
}

void ASphericalTerrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FPlayerViewInfo viewInfo = GetCurrentViewInfo(GetWorld());
	for (USphericalTerrainFace* face : TerrainFaces)
	{
		if (!face) continue;
		face->ComputeLOD(viewInfo);
	}
}

void ASphericalTerrain::InitializeTextureArrays()
{
	if (bTextureArraysInitialized) return;

	UE_LOG(LogTemp, Log, TEXT("Initializing Texture Arrays for Spherical Terrain..."));

	// Height Map Array
	HeightMapArray = UTexture2DArray::CreateTransient(
		TextureArrayResolution,
		TextureArrayResolution,
		MaxSimultaneousChunks,
		PF_R32_FLOAT);

	HeightMapArray->AddressX = TA_Clamp;
	HeightMapArray->AddressY = TA_Clamp;
	HeightMapArray->Filter = TF_Nearest;
	HeightMapArray->SRGB = false;
	HeightMapArray->UpdateResource();

	// Color Map Array
	ColorMapArray = UTexture2DArray::CreateTransient(
		TextureArrayResolution,
		TextureArrayResolution,
		MaxSimultaneousChunks,
		PF_B8G8R8A8);

	ColorMapArray->AddressX = TA_Clamp;
	ColorMapArray->AddressY = TA_Clamp;
	ColorMapArray->Filter = TF_Bilinear;
	ColorMapArray->SRGB = true;
	ColorMapArray->UpdateResource();

	// Normal Map Array
	NormalMapArray = UTexture2DArray::CreateTransient(
		TextureArrayResolution,
		TextureArrayResolution,
		MaxSimultaneousChunks,
		PF_B8G8R8A8);

	NormalMapArray->AddressX = TA_Clamp;
	NormalMapArray->AddressY = TA_Clamp;
	NormalMapArray->Filter = TF_Bilinear;
	NormalMapArray->SRGB = false;
	NormalMapArray->UpdateResource();

	// Initialize the slice pool
	AvailableSlices.Reserve(MaxSimultaneousChunks);
	for (int32 i = 0; i < MaxSimultaneousChunks; ++i)
	{
		AvailableSlices.Add(i);
	}

	SliceToChunk.Reserve(MaxSimultaneousChunks);

	// Creating shared material instance
	if (TerrainMaterial)
	{
		SharedTerrainMaterial = UMaterialInstanceDynamic::Create(TerrainMaterial, this);
		SharedTerrainMaterial->SetTextureParameterValue(TEXT("HeightMapArray"), HeightMapArray);
		SharedTerrainMaterial->SetTextureParameterValue(TEXT("ColorMapArray"), ColorMapArray);
		SharedTerrainMaterial->SetTextureParameterValue(TEXT("NormalMapArray"), NormalMapArray);
		SharedTerrainMaterial->SetScalarParameterValue(TEXT("PlanetRadius"), Radius);
		SharedTerrainMaterial->SetVectorParameterValue(TEXT("PlanetCenter"), GetActorLocation());
		SharedTerrainMaterial->SetScalarParameterValue(TEXT("HeightScale"), 1.0f);
	}

	bTextureArraysInitialized = true;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("Texture Arrays initialized with resolution %dx%d and %d slices."),
		TextureArrayResolution,
		TextureArrayResolution,
		MaxSimultaneousChunks);
}

int32 ASphericalTerrain::AllocateSlice(USphericalTerrainFace* Chunk)
{
	if (AvailableSlices.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No available texture array slices! Evicting least important chunk..."));

		FPlayerViewInfo ViewInfo = GetCurrentViewInfo(GetWorld());
		EvictLeastImportantChunk(ViewInfo.CameraPosition);

		if (AvailableSlices.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to allocate texture array slice after eviction!"));
			return -1;
		}
	}

	int32 SliceIndex = AvailableSlices.Pop();
	SliceToChunk.Add(SliceIndex, Chunk);

	return SliceIndex;
}

void ASphericalTerrain::FreeSlice(int32 SliceIndex)
{
	if (SliceIndex >= 0 && SliceIndex < MaxSimultaneousChunks)
	{
		AvailableSlices.Add(SliceIndex);
		SliceToChunk.Remove(SliceIndex);
	}
}

void ASphericalTerrain::EvictLeastImportantChunk(const FVector& CameraPosition)
{
	USphericalTerrainFace* FarthestChunk = nullptr;
	double MaxDistance = -1.0;

	for (const auto& Pair : SliceToChunk)
	{
		USphericalTerrainFace* Chunk = Pair.Value;
		if (!Chunk) continue;

		double Distance = FVector::Dist(Chunk->GetCenter(), CameraPosition);
		if (Distance > MaxDistance)
		{
			MaxDistance = Distance;
			FarthestChunk = Chunk;
		}
	}

	if (FarthestChunk)
	{
		UE_LOG(LogTemp, Log, TEXT("Evicting chunk at distance %.2f"), MaxDistance);
		FarthestChunk->DestroyChunkInstance();
	}
}

void ASphericalTerrain::GenerateTerrainIntoSlice(int32 SliceIndex, const struct FTerrainNoiseParams& NoiseParams)
{
	if (SliceIndex < 0 || SliceIndex >= MaxSimultaneousChunks)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid slice index %d"), SliceIndex);
		return;
	}

	//FTerrainNoiseDispatcher::DispatchToTextureArraySlice(
	//	HeightMapArray,
	//	ColorMapArray,
	//	NormalMapArray,
	//	SliceIndex,
	//	NoiseParams);
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
			face->Frequency = Frequency;
			face->LODLevel = 0;

			switch (face->FaceDirection)
			{
			case ETerrainFaceDirection::Up:
				{
					FBox limitsUp(FVector(-Radius, -Radius, Radius), FVector(Radius, Radius, Radius));
					face->Limits = limitsUp;
				}
				break;

			case ETerrainFaceDirection::Down:
				{
					FBox limitsDown(FVector(-Radius, -Radius, -Radius), FVector(Radius, Radius, -Radius));
					face->Limits = limitsDown;
				}
				break;

			case ETerrainFaceDirection::Left:
				{
					FBox limitsLeft(FVector(Radius, -Radius, -Radius), FVector(Radius, Radius, Radius));
					face->Limits = limitsLeft;
				}
				break;

			case ETerrainFaceDirection::Right:
				{
					FBox limitsRight(FVector(-Radius, -Radius, -Radius), FVector(-Radius, Radius, Radius));
					face->Limits = limitsRight;
				}
				break;

			case ETerrainFaceDirection::Forward:
				{
					FBox limitsForward(FVector(-Radius, Radius, -Radius), FVector(Radius, Radius, Radius));
					face->Limits = limitsForward;
				}
				break;

			case ETerrainFaceDirection::Backward:
				{
					FBox limitsBackward(FVector(-Radius, -Radius, -Radius), FVector(Radius, -Radius, Radius));
					face->Limits = limitsBackward;
				}
				break;
			}

			face->OnBuild();
		}
	}
}

#if WITH_EDITOR
void ASphericalTerrain::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.GetPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASphericalTerrain, MaxSimultaneousChunks) || PropertyName ==
		GET_MEMBER_NAME_CHECKED(ASphericalTerrain, TextureArrayResolution))
	{
		bTextureArraysInitialized = false;
		InitializeTextureArrays();
	}

	SetupFaces();
}
#endif
