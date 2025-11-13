// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0.
// Noncommercial use only. Commercial use requires written permission.
// See https://polyformproject.org/licenses/noncommercial/1.0.0/
#include "PlanetData.h"

UPlanetData::UPlanetData()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlanetData::InitializeDataLayers()
{
	if (!Grid)
	{
		UE_LOG(LogTemp, Error, TEXT("UPlanetData::InitializeDataLayers - Grid is nullptr."));
		return;
	}

	int32 cellCount = Grid->TotalCellCount;

	// Initialize all data layers
	ElevationLevel.SetNumZeroed(cellCount);
	CellTemperature.SetNumZeroed(cellCount);
	CellHumidity.SetNumZeroed(cellCount);
	Biome.SetNumZeroed(cellCount);
	TectonicPlateId.SetNumZeroed(cellCount);
	CellRegionId.SetNumZeroed(cellCount);

	UE_LOG(LogTemp, Log, TEXT("UPlanetData::InitializeDataLayers - Data layers initialized for %d cells."), cellCount);
}

void UPlanetData::ClearDataLayers()
{
	ElevationLevel.Empty();
	CellTemperature.Empty();
	CellHumidity.Empty();
	Biome.Empty();
	TectonicPlateId.Empty();
	CellRegionId.Empty();
}

int32 UPlanetData::FindCellAtPosition(const FVector& Position) const
{
	if (!Grid || !GetOwner())
	{
		return INDEX_NONE;
	}

	// Convert to local space
	FVector LocalPos = Position - GetOwner()->GetActorLocation();
	FVector Direction = LocalPos.GetSafeNormal();

	return Grid->FindCellAtPosition(Direction);
}

FVector UPlanetData::CellIdToWorldPosition(int32 CellId) const
{
	if (!IsValidCellId(CellId) || !GetOwner())
	{
		return FVector::ZeroVector;
	}

	const FHexCell& Cell = Grid->Cells[CellId];
	FVector worldPos = Cell.Position * PlanetRadius;
	worldPos += GetOwner()->GetActorLocation();

	return worldPos;
}

int32 UPlanetData::GetCellElevation(int32 CellId) const
{
	return IsValidCellId(CellId) ? ElevationLevel[CellId] : 0;
}

void UPlanetData::SetCellElevation(int32 CellId, int32 Elevation)
{
	if (IsValidCellId(CellId))
	{
		ElevationLevel[CellId] = FMath::Clamp(Elevation, MIN_ELEVATION_LEVEL, MAX_ELEVATION_LEVEL);
	}
}

int32 UPlanetData::GetWaterDepth(int32 CellId) const
{
	if (IsValidCellId(CellId))
	{
		int32 Elevation = ElevationLevel[CellId];
		if (Elevation < WaterLevel)
		{
			return WaterLevel - Elevation;
		}
	}
	return 0;
}

float UPlanetData::GetCellTemperature(int32 CellId) const
{
	return IsValidCellId(CellId) ? CellTemperature[CellId] : 0.0f;
}

void UPlanetData::SetCellTemperature(int32 CellId, float InTemperature)
{
	if (IsValidCellId(CellId))
	{
		CellTemperature[CellId] = InTemperature;
	}
}

float UPlanetData::GetCellHumidity(int32 CellId) const
{
	return IsValidCellId(CellId) ? CellHumidity[CellId] : 0.0f;
}

void UPlanetData::SetCellHumidity(int32 CellId, float InHumidity)
{
	if (IsValidCellId(CellId))
	{
		CellHumidity[CellId] = InHumidity;
	}
}

UBiomeData* UPlanetData::GetCellBiome(int32 CellId) const
{
	return IsValidCellId(CellId) ? Biome[CellId] : nullptr;
}

void UPlanetData::SetCellBiome(int32 CellId, UBiomeData* BiomeData)
{
	if (IsValidCellId(CellId))
	{
		Biome[CellId] = BiomeData;
	}
}

int32 UPlanetData::GetCellTectonicPlateId(int32 CellId) const
{
	return IsValidCellId(CellId) ? TectonicPlateId[CellId] : -1;
}

void UPlanetData::SetCellTectonicPlateId(int32 CellId, int32 PlateId)
{
	if (IsValidCellId(CellId))
	{
		TectonicPlateId[CellId] = PlateId;
	}
}

int32 UPlanetData::GetCellRegionId(int32 CellId) const
{
	return IsValidCellId(CellId) ? CellRegionId[CellId] : -1;
}

void UPlanetData::SetCellRegionId(int32 CellId, int32 InRegionId)
{
	if (IsValidCellId(CellId))
	{
		CellRegionId[CellId] = InRegionId;
	}
}

bool UPlanetData::IsCellUnderwater(int32 CellId) const
{
	return IsValidCellId(CellId) && (ElevationLevel[CellId] <= WaterLevel);
}

bool UPlanetData::IsValidCellId(int32 CellId) const
{
	return Grid != nullptr && CellId < Grid->Cells.Num() && CellId >= 0;
}

int32 UPlanetData::GetCellCount() const
{
	return Grid != nullptr ? Grid->TotalCellCount : 0;
}
