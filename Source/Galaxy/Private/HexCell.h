// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0.
// Noncommercial use only. Commercial use requires written permission.
// See https://polyformproject.org/licenses/noncommercial/1.0.0/
#pragma once

#include "CoreMinimal.h"
#include "HexCell.generated.h"

/**
* Type of cell in the hexagonal grid
*/
UENUM(BlueprintType)
enum class EHexCellType : uint8
{
	/** Standard hexagonal cell with 6 neighbors */
	Hexagon = 0,
	/** Pentagonal cell with 5 neighbors */
	Pentagon = 1
};

/**
 * Represents a single cell in a hexagonal grid on a spherical surface
 */
USTRUCT(BlueprintType)
struct GALAXY_API FHexCell
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HexCell")
	int32 CellId = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HexCell")
	EHexCellType CellType = EHexCellType::Hexagon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HexCell")
	FVector Position = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, Category = "HexCell")
	TArray<uint32> NeighborCellIds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HexCell")
	TArray<FVector> Vertices;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HexCell")
	uint8 IcosaheronFaceIndex = 0;

	FHexCell();

	int32 GetNeighborCount() const;
	bool IsPentagon() const;
	bool IsHexagon() const;
	int32 GetNeighborByIndex(int32 Index) const;
	bool HasNeighbor(int32 NeighborCellId) const;
	float CalculateArea(float SphereRadius) const;
	FVector GetVertexCentroid() const;
};
