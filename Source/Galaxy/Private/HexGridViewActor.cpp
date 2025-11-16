// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0.
// Noncommercial use only. Commercial use requires written permission.
// See https://polyformproject.org/licenses/noncommercial/1.0.0/
#include "HexGridViewActor.h"
#include "HexGridGenerator.h"


// Sets default values
AHexGridViewActor::AHexGridViewActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

#if WITH_EDITOR
	bRunConstructionScriptOnDrag = true;
	SetCanBeDamaged(false);
#endif
}

void AHexGridViewActor::BeginPlay()
{
	Super::BeginPlay();
}

void AHexGridViewActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if WITH_EDITOR
	if (GetWorld() && GetWorld()->WorldType == EWorldType::Editor)
	{
		DrawGrid();
	}
#endif
}

#if WITH_EDITOR
void AHexGridViewActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Refresh display when properties change
	if (PropertyChangedEvent.Property)
	{
		FName propertyName = PropertyChangedEvent.Property->GetFName();

		// Force a redraw
		if (GetWorld())
		{
			GetWorld()->bRequestedBlockOnAsyncLoading = false;
		}
	}
}
#endif

void AHexGridViewActor::GenerateGrid(int32 level)
{
	TArray<FString> errors;
	GridAsset = UHexGridGenerator::GenerateHexGrid(level, errors);

	if (errors.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AHexGridPreviewActor::GenerateGrid - Errors during generation:"));
		for (const FString& error : errors)
		{
			UE_LOG(LogTemp, Warning, TEXT(" - % s"), *error);
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("AHexGridPreviewActor::GenerateGrid - Successfully generated level %d grid."), level);
	}
}

void AHexGridViewActor::SelectCell(int32 cellId)
{
	if (GridAsset && cellId >= 0 && cellId < GridAsset->Cells.Num())
	{
		selectedCellID = cellId;
		UE_LOG(LogTemp, Log, TEXT("Selected cell %d: %s"), cellId, *GetSelectedCellInfo());
	}
	else
	{
		selectedCellID = INDEX_NONE;
	}
}

void AHexGridViewActor::SelectCellAtPosition(FVector worldPosition)
{
	if (!GridAsset)
	{
		return;
	}

	FVector localPos = worldPosition - GetActorLocation();
	FVector direction = localPos.GetSafeNormal();

	uint32 cellId = GridAsset->FindCellAtPosition(direction);
	SelectCell(static_cast<int32>(cellId));
}

void AHexGridViewActor::ClearSelection()
{
	selectedCellID = INDEX_NONE;
}

FString AHexGridViewActor::GetSelectedCellInfo() const
{
	if (!GridAsset || selectedCellID == INDEX_NONE || selectedCellID >= GridAsset->Cells.Num())
	{
		return TEXT("No cell selected");
	}

	const FHexCell& cell = GridAsset->Cells[selectedCellID];

	FString info = FString::Printf(TEXT("Cell %d\n"), cell.CellId);
	info += FString::Printf(TEXT("Type: %s\n"), cell.IsPentagon() ? TEXT("Pentagon") : TEXT("Hexagon"));
	info += FString::Printf(TEXT("Position: (%.2f, %.2f, %.2f)\n"), cell.Position.X, cell.Position.Y, cell.Position.Z);
	info += FString::Printf(TEXT("Icosahedron Face: %d\n"), cell.IcosaheronFaceIndex);
	info += FString::Printf(TEXT("Neighbors: "));

	for (int32 i = 0; i < cell.NeighborCellIds.Num(); ++i)
	{
		info += FString::Printf(TEXT("%d "), cell.NeighborCellIds[i]);
		if(i < cell.NeighborCellIds.Num() - 1)
		{
			info += TEXT(", ");
		}
	}

	info += TEXT("\n");
	info += FString::Printf(TEXT("Vertices: %d\n"), cell.Vertices.Num());
	info += FString::Printf(TEXT("Area: %.2f\n"), cell.CalculateArea(displayRadius));

	return info;
}

void AHexGridViewActor::DrawGrid()
{
	if (!GridAsset || !GetWorld())
	{
		return;
	}

	// Draw all cells
	for (const FHexCell& cell : GridAsset->Cells)
	{
		if (!ShouldDrawCell(cell))
		{
			continue;
		}

		bool bIsSelected = (cell.CellId == selectedCellID);
		DrawCell(cell, bIsSelected);
	}

	// Draw statistics
	if (bShowStatistics)
	{
		DrawStatistics();
	}
}

void AHexGridViewActor::DrawCell(const FHexCell& cell, bool bIsSelected /* = false */)
{
	FLinearColor cellColor = bIsSelected && bHighlightSelected ? selectedCellColor : GetCellColor(cell);

	// Draw outline
	if (bShowCellOutlines)
	{
		DrawCellOutline(cell, cellColor);
	}

	// Draw center
	if (bShowCellCenters)
	{
		DrawCellCenter(cell, cellColor);
	}

	// Draw neighbor connections
	if (bShowNeighborConnections && (bIsSelected || !bHighlightSelected))
	{
		DrawNeighborConnections(cell);
	}

	// Draw cell ID
	if (bShowCellIDs && (bIsSelected || !bHighlightSelected))
	{
		DrawCellID(cell);
	}
}

void AHexGridViewActor::DrawCellOutline(const FHexCell& cell, FLinearColor color)
{
	if (cell.Vertices.Num() < 3)
	{
		return;
	}

	FVector actorLoc = GetActorLocation();

	for (int32 i = 0; i < cell.Vertices.Num(); ++i)
	{
		int32 nextIdx = (i + 1) % cell.Vertices.Num();

		FVector start = GridToWorldPosition(cell.Vertices[i]);
		FVector end = GridToWorldPosition(cell.Vertices[nextIdx]);

		DrawDebugLine(GetWorld(), start, end, color.ToFColor(true), false, -1.0f, 0, outlineThickness);
	}
}

void AHexGridViewActor::DrawCellCenter(const FHexCell& cell, FLinearColor color)
{

}

void AHexGridViewActor::DrawNeighborConnections(const FHexCell& cell)
{

}

void AHexGridViewActor::DrawCellID(const FHexCell& cell)
{

}

void AHexGridViewActor::DrawStatistics()
{

}

FLinearColor AHexGridViewActor::GetCellColor(const FHexCell& cell) const
{
	// Color by type (hex vs pentagon)
	if (bHighlightSelected && cell.IsPentagon())
	{
		return pentagonColor;
	}

	// Color by icosahedron face
	if (bColorByIcosahedronFaces)
	{
		return GetFaceColor(cell.IcosaheronFaceIndex);
	}

	return hexagonColor;
}

bool AHexGridViewActor::ShouldDrawCell(const FHexCell& cell) const
{
	// Filter by cell ID range
	if (bUseCellIDRange)
	{
		if(cell.CellId < minCellID || cell.CellId > maxCellID)
		{
			return false;
		}
	}

	// Filter by icosahedron face
	if (bFilterByIcosahedronFace)
	{
		if (cell.IcosaheronFaceIndex != icosahedronFaceFilter)
		{
			return false;
		}
	}

	return true;
}

FVector AHexGridViewActor::GridToWorldPosition(const FVector& gridPosition) const
{
	return GetActorLocation() + gridPosition * displayRadius;
}

FLinearColor AHexGridViewActor::GetFaceColor(uint8 faceIndex) const
{
	// Generate 20 distinct colors for the icosahedron faces
	static const TArray<FLinearColor> faceColors = {
		FLinearColor::Red,
		FLinearColor::Green,
		FLinearColor::Blue,
		FLinearColor::Yellow,
		FLinearColor(1.0f, 0.0f, 1.0f), // Magenta
		FLinearColor(0.0f, 1.0f, 1.0f), // Cyan
		FLinearColor(1.0f, 0.5f, 0.0f), // Orange
		FLinearColor(0.5f, 0.0f, 1.0f), // Purple
		FLinearColor(0.0f, 1.0f, 0.5f), // Spring Green
		FLinearColor(1.0f, 0.0f, 0.5f), // Rose
		FLinearColor(0.5f, 1.0f, 0.0f), // Chartreuse
		FLinearColor(0.0f, 0.5f, 1.0f), // Sky Blue
		FLinearColor(1.0f, 1.0f, 0.5f), // Light Yellow
		FLinearColor(1.0f, 0.5f, 1.0f), // Light Magenta
		FLinearColor(0.5f, 1.0f, 1.0f), // Light Cyan
		FLinearColor(0.5f, 0.5f, 0.0f), // Olive
		FLinearColor(0.5f, 0.0f, 0.5f), // Dark Magenta
		FLinearColor(0.0f, 0.5f, 0.5f), // Teal
		FLinearColor(1.0f, 0.75f, 0.5f), // Peach
		FLinearColor(0.75f, 0.5f, 1.0f), // Lavender
	};

	if (faceIndex < faceColors.Num())
	{
		return faceColors[faceIndex];
	}

	return FLinearColor::White;
}

