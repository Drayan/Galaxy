// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0.
// Noncommercial use only. Commercial use requires written permission.
// See https://polyformproject.org/licenses/noncommercial/1.0.0/
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HexGridAsset.h"
#include "BiomeData.h"
#include "PlanetData.generated.h"

UCLASS(classGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GALAXY_API UPlanetData : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlanetData();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Setup")
	TObjectPtr<UHexGridAsset> Grid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Setup")
	float PlanetRadius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Setup", meta = (ClampMin = "-5", ClampMax = "5"))
	int32 WaterLevel = 0;

	// === DATA LAYERS ===
	// All layers are indexed by CellId from the grid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Data|Geophysical")
	TArray<int32> ElevationLevel;

	static constexpr int32 MIN_ELEVATION_LEVEL = -5;
	static constexpr int32 MAX_ELEVATION_LEVEL = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Data|Climate")
	TArray<float> CellTemperature;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Data|Climate")
	TArray<float> CellHumidity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Data|Terrain")
	TArray<TObjectPtr<UBiomeData>> Biome;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Data|Geophysical")
	TArray<int32> TectonicPlateId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Data|Gameplay")
	TArray<int32> CellRegionId;

	// === INITIALIZATION METHODS ===
	UFUNCTION(BlueprintCallable, Category = "Planet Data")
	void InitializeDataLayers();

	UFUNCTION(BlueprintCallable, Category = "Planet Data")
	void ClearDataLayers();

	// === DATA ACCESS METHODS ===
	UFUNCTION(BlueprintCallable, Category = "Planet Data")
	int32 GetCellElevation(int32 CellId) const;

	UFUNCTION(BlueprintCallable, Category = "Planet Data")
	void SetCellElevation(int32 CellId, int32 elevation);

	UFUNCTION(BlueprintCallable, Category = "Planet Data")
	int32 GetWaterDepth(int32 CellId) const;

	UFUNCTION(BlueprintCallable, Category = "Planet Data")
	float GetCellTemperature(int32 CellId) const;

	UFUNCTION(BlueprintCallable, Category = "Planet Data")
	void SetCellTemperature(int32 CellId, float temperature);

	UFUNCTION(BlueprintCallable, Category = "Planet Data")
	float GetCellHumidity(int32 CellId) const;

	UFUNCTION(BlueprintCallable, Category = "Planet Data")
	void SetCellHumidity(int32 CellId, float humidity);

	UFUNCTION(BlueprintCallable, Category = "Planet Data")
	UBiomeData* GetCellBiome(int32 CellId) const;

	UFUNCTION(BlueprintCallable, Category = "Planet Data")
	void SetCellBiome(int32 CellId, UBiomeData* BiomeData);

	UFUNCTION(BlueprintCallable, Category = "Planet Data")
	int32 GetCellTectonicPlateId(int32 CellId) const;

	UFUNCTION(BlueprintCallable, Category = "Planet Data")
	void SetCellTectonicPlateId(int32 CellId, int32 PlateId);

	UFUNCTION(BlueprintCallable, Category = "Planet Data")
	int32 GetCellRegionId(int32 CellId) const;

	UFUNCTION(BlueprintCallable, Category = "Planet Data")
	void SetCellRegionId(int32 CellId, int32 regionId);

	// === UTILITY METHODS ===
	UFUNCTION(BlueprintCallable, Category = "Planet Data")
	bool IsCellUnderwater(int32 CellId) const;

	UFUNCTION(BlueprintCallable, Category = "Planet Data")
	bool IsValidCellId(int32 CellId) const;

	UFUNCTION(BlueprintCallable, Category = "Planet Data")
	int32 GetCellCount() const;

	UFUNCTION(BlueprintCallable, Category = "Planet Data")
	int32 FindCellAtPosition(const FVector& Position) const;

	UFUNCTION(BlueprintCallable, Category = "Planet Data")
	FVector CellIdToWorldPosition(int32 CellId) const;
};
