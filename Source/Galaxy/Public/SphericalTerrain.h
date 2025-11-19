// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0.
// Noncommercial use only. Commercial use requires written permission.
// See https://polyformproject.org/licenses/noncommercial/1.0.0/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SphericalTerrain.generated.h"

class USphericalTerrainFace;
class UTerrainMeshPool;
class UMaterialInterface;
class UTexture2DArray;

USTRUCT(BlueprintType)
struct FPlayerViewInfo
{
	GENERATED_BODY()

	FVector CameraPosition = FVector::ZeroVector;
	double FOVY_Rad = 1.0;
	int32 ScreenHeight = 1080;
};

// Helper function
FPlayerViewInfo GetCurrentViewInfo(UWorld* World);

UCLASS()
class GALAXY_API ASphericalTerrain : public AActor
{
	GENERATED_BODY()
	
public:	
	ASphericalTerrain();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	double Radius = 500.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	int32 MeshResolution = 64;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	int32 MaxLOD = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	float Frequency = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|TextureArray")
	int32 MaxSimultaneousChunks = 256;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|TextureArray")
	int32 TextureArrayResolution = 512;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	UMaterialInterface* TerrainMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	TSubclassOf<UTerrainMeshPool> MeshPool = nullptr;

	UPROPERTY(VisibleAnywhere)
	TArray<USphericalTerrainFace*> TerrainFaces;

	UPROPERTY(VisibleAnywhere)
	UTexture2DArray* HeightMapArray = nullptr;

	UPROPERTY(VisibleAnywhere)
	UTexture2DArray* ColorMapArray = nullptr;

	UPROPERTY(VisibleAnywhere)
	UTexture2DArray* NormalMapArray = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* SharedTerrainMaterial = nullptr;

	void InitializeTextureArrays();
	int32 AllocateSlice(USphericalTerrainFace* Chunk);
	void FreeSlice(int32 SliceIndex);
	void GenerateTerrainIntoSlice(int32 SliceIndex, const struct FTerrainNoiseParams& NoiseParams);

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	void SetupFaces();
	void EvictLeastImportantChunk(const FVector& CameraPosition);

	TArray<int32> AvailableSlices;

	TMap<int32, USphericalTerrainFace*> SliceToChunk;

	bool bTextureArraysInitialized = false;
};

