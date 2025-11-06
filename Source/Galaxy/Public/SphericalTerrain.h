// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0.
// Noncommercial use only. Commercial use requires written permission.
// See https://polyformproject.org/licenses/noncommercial/1.0.0/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SphericalTerrainFace.h"
#include "SphericalTerrain.generated.h"

struct FPlayerViewInfo
{
	FVector CameraPosition;
	double FOVY_Rad;
	int32 ScreenHeight;
};

UCLASS()
class GALAXY_API ASphericalTerrain : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASphericalTerrain();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	UMaterialInterface* TerrainMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	double Radius = 500.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	int32 MeshResolution = 32;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	int32 HeightMapResolution = 512;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	int32 MaxLOD = 6;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	UPROPERTY(VisibleAnywhere)
	TArray<USphericalTerrainFace*> TerrainFaces;

	void SetupFaces();
};
