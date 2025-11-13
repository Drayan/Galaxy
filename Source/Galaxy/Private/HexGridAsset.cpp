// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0.
// Noncommercial use only. Commercial use requires written permission.
// See https://polyformproject.org/licenses/noncommercial/1.0.0/
#include "HexGridAsset.h"

const FHexCell& UHexGridAsset::GetCellById(int32 CellId) const
{
	if(CellId >= 0 && CellId < Cells.Num())
	{
		return Cells[CellId];
	}

	static FHexCell InvalidCell;
	return InvalidCell;
}

void UHexGridAsset::GetNeighbors(int32 CellId, TArray<int32>& outNeighborIds) const
{
	if(CellId >= 0 && CellId < Cells.Num())
	{
		outNeighborIds.Reset();
		outNeighborIds.Append(Cells[CellId].NeighborCellIds);
	}
}

int32 UHexGridAsset::FindCellAtPosition(const FVector& Position) const
{
	if (Cells.Num() == 0)
	{
		return INDEX_NONE;
	}

	FVector normalizedPos = Position.GetSafeNormal();

	uint32 closestCellId = 0;
	float minDistanceSq = FLT_MAX;
	for (int32 i = 0; i < Cells.Num(); ++i)
	{
		float distanceSq = FVector::DistSquared(normalizedPos, Cells[i].Position);
		if (distanceSq < minDistanceSq)
		{
			minDistanceSq = distanceSq;
			closestCellId = i;
		}
	}

	return closestCellId;
}

void UHexGridAsset::FindClosestCells(const FVector& Position, TArray<int32>& outCells, int32 Count /*= 3*/) const
{
	if(Cells.Num() == 0 || Count <= 0)
	{
		return;
	}

	FVector normalizedPos = Position.GetSafeNormal();

	// Create array of (cellId, distanceSq) pairs
	TArray<TPair<uint32, float>> cellDistances;
	cellDistances.Reserve(Cells.Num());

	for (int32 i = 0; i < Cells.Num(); ++i)
	{
		float distanceSq = FVector::DistSquared(normalizedPos, Cells[i].Position);
		cellDistances.Add(TPair<uint32, float>(i, distanceSq));
	}

	// Sort by distance
	cellDistances.Sort([](const TPair<uint32, float>& A, const TPair<uint32, float>& B)
	{
		return A.Value < B.Value;
		});

	// Take the first 'Count' cell IDs
	int32 numToTake = FMath::Min(Count, cellDistances.Num());
	outCells.Reset(numToTake);
	for (int32 i = 0; i < numToTake; ++i)
	{
		outCells.Add(cellDistances[i].Key);
	}
}

void UHexGridAsset::FindCellsInRadius(FVector position, int32 Radius, TArray<int32>& outCells) const
{
	if(Cells.Num() == 0)
	{
		return;
	}

	FVector normalizedPos = position.GetSafeNormal();
	float radiusSq = Radius * Radius;
	outCells.Reset();

	for(int32 i = 0; i < Cells.Num(); ++i)
	{
		float distanceSq = FVector::DistSquared(normalizedPos, Cells[i].Position);
		if(distanceSq <= radiusSq)
		{
			outCells.Add(i);
		}
	}
}

const TArray<int32>& UHexGridAsset::GetPentagons() const
{
	return PentagonCellsIds;
}

bool UHexGridAsset::ValidateGrid(TArray<FString>& outErrors) const
{
	outErrors.Empty();
	bool bIsValid = true;

	// Check pentagon count
	if (PentagonCount != 12)
	{
		outErrors.Add(FString::Printf(TEXT("Invalid pentagon count: expected 12, found %d"), PentagonCount));
		bIsValid = false;
	}

	// Check total cell count
	if (TotalCellCount != Cells.Num())
	{
		outErrors.Add(FString::Printf(TEXT("Invalid total cell count: expected %d, found %d"), TotalCellCount, Cells.Num()));
		bIsValid = false;
	}

	// Validate each cell
	for (int32 i = 0; i < Cells.Num(); ++i)
	{
		const FHexCell& cell = Cells[i];

		// Check cell ID matches index
		if (cell.CellId != i)
		{
			outErrors.Add(FString::Printf(TEXT("Cell ID mismatch at index %d: found %d"), i, cell.CellId));
			bIsValid = false;
		}

		// Check neighbor count
		int32 expectedNeighborCount = cell.GetNeighborCount();
		if (cell.NeighborCellIds.Num() != expectedNeighborCount)
		{
			outErrors.Add(FString::Printf(TEXT("Neighbor count mismatch at index %d: expected %d, found %d"), i, expectedNeighborCount, cell.NeighborCellIds.Num()));
			bIsValid = false;
		}

		// Check neighbor symmetry
		for (int32 neighborId : cell.NeighborCellIds)
		{
			if (neighborId >= i)
			{
				outErrors.Add(FString::Printf(TEXT("Neighbor symmetry mismatch at index %d: found neighbor %d"), i, neighborId));
				bIsValid = false;
				continue;
			}

			const FHexCell& neighborCell = Cells[neighborId];
			if (!neighborCell.HasNeighbor(i))
			{
				outErrors.Add(FString::Printf(TEXT("Neighbor symmetry mismatch between cells %d and %d"), i, neighborId));
				bIsValid = false;
			}
		}

		// Check position normalization
		if (!FMath::IsNearlyEqual(cell.Position.Size(), 1.0f, 0.001f))
		{
			outErrors.Add(FString::Printf(TEXT("Cell %d is not normalized, length=%f"), i, cell.Position.Size()));
			bIsValid = false;
		}
	}

	return bIsValid;
}

void UHexGridAsset::CalculateStatistics()
{
	if (Cells.Num() == 0)
	{
		return;
	}

	TArray<float> areas;
	areas.Reserve(Cells.Num());

	for (const FHexCell& cell : Cells)
	{
		float area = cell.CalculateArea(1.0f); // Assuming unit sphere radius
		areas.Add(area);
	}

	// Find min and max
	MinCellArea = areas[0];
	MaxCellArea = areas[0];
	float sum = 0.0f;

	for (float area : areas)
	{
		MinCellArea = FMath::Min(MinCellArea, area);
		MaxCellArea = FMath::Max(MaxCellArea, area);
		sum += area;
	}

	AverageCellArea = sum / areas.Num();

	// Calculate standard deviation
	float varianceSum = 0.0f;
	for (float area : areas)
	{
		float diff = area - AverageCellArea;
		varianceSum += diff * diff;
	}
	AreaStandardDeviation = FMath::Sqrt(varianceSum / areas.Num());
}

int32 UHexGridAsset::GetExpectedCellCount(int32 Level)
{
	// Formula: 10 * 4^Level + 2
	// (or equivalently: 20 * 4^(Level - 1) + 2 for triangles, then dual)
	//int32 triangleCount = 20 * FMath::Pow(4, Level - 1);

	// For dual grid: vertices of triangle mesh become cells
	// Base icosahedron has 12 vertices, each subdivision adds new vertices
	int32 baseVertices = 12;
	int32 edgeVertices = 30 * (FMath::Pow(2.0f, Level) - 1); // edges subdivided
	int32 faceVertices = 20 * ((FMath::Pow(2.0f, Level) - 1) * (FMath::Pow(2.0f, Level) - 2)) / 2; // faces subdivided

	return baseVertices + edgeVertices + faceVertices;
}
