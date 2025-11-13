// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0.
// Noncommercial use only. Commercial use requires written permission.
// See https://polyformproject.org/licenses/noncommercial/1.0.0/
#include "HexCell.h"

FHexCell::FHexCell()
{
	NeighborCellIds.Reserve(6);
	Vertices.Reserve(6);
}

int32 FHexCell::GetNeighborCount() const
{
	return CellType == EHexCellType::Pentagon ? 5 : 6;
}

bool FHexCell::IsPentagon() const
{
	return CellType == EHexCellType::Pentagon;
}

bool FHexCell::IsHexagon() const
{
	return CellType == EHexCellType::Hexagon;
}

int32 FHexCell::GetNeighborByIndex(int32 Index) const
{
	if(Index >= 0 && Index < NeighborCellIds.Num())
	{
		return NeighborCellIds[Index];
	}

	return UINT32_MAX;
}

bool FHexCell::HasNeighbor(int32 NeighborCellId) const
{
	return NeighborCellIds.Contains(NeighborCellId);
}

float FHexCell::CalculateArea(float SphereRadius) const
{
	if(Vertices.Num() < 3)
	{
		return 0.0f;
	}

	// Use spherical excess formula for spherical polygons
	float totalArea = 0.0f;
	FVector center = Position * SphereRadius;

	for (int32 i = 0; i < Vertices.Num(); ++i)
	{
		int32 nextIndex = (i + 1) % Vertices.Num();
		FVector v1 = Vertices[i] * SphereRadius;
		FVector v2 = Vertices[nextIndex] * SphereRadius;

		// Area of the spherical triangle formed by center, v1, and v2
		// Simplified using cross product magnitude / 2
		FVector edge1 = v1 - center;
		FVector edge2 = v2 - center;
		float triangleArea = FVector::CrossProduct(edge1, edge2).Size() / 2.0f;
		totalArea += triangleArea;
	}

	return totalArea;
}

FVector FHexCell::GetVertexCentroid() const
{
	if(Vertices.Num() == 0)
	{
		return Position;
	}

	FVector sum = FVector::ZeroVector;
	for(const FVector& vertex : Vertices)
	{
		sum += vertex;
	}

	return (sum / Vertices.Num()).GetSafeNormal();
}