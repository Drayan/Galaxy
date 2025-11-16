// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0.
// Noncommercial use only. Commercial use requires written permission.
// See https://polyformproject.org/licenses/noncommercial/1.0.0/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HexGridAsset.h"
#include "HexGridViewActor.generated.h"

/// <summary>
/// Visualization mode for hex grid display
/// </summary>
UENUM(BlueprintType)
enum class EHexGridVisualizationMode : uint8
{
	CellOutlines		UMETA(DisplayName = "Cell Outlines"),
	CellCenters			UMETA(DisplayName = "Cell Centers"),
	ByType				UMETA(DisplayName = "By Type"),
	ByIcosahedronFaces	UMETA(DisplayName = "By Icosahedron Faces"),
	NeighborConnections	UMETA(DisplayName = "Neighbor Connections"),
	CellIDs				UMETA(DisplayName = "Cell IDs"),
};

/// <summary>
/// Actor for previewing and debugging hex grids in the editor
/// Place this in the level and assign a HexGridAsset to visualize it
/// </summary>
UCLASS(hideCategories = (Input, Collision, Replication, Rendering, Actor, LOD, Cooking))
class GALAXY_API AHexGridViewActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AHexGridViewActor();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex Grid Preview")
	TObjectPtr<UHexGridAsset> GridAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hex Grid Preview", Meta = (ClampMin = "100.0", ClampMax = "1000000.0"))
	float displayRadius = 10000.0f;

	// === VISUALIZATION OPTIONS ===

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid Preview|Display")
	bool bShowCellOutlines = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid Preview|Display", Meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float outlineThickness = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid Preview|Display")
	bool bShowCellCenters = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid Preview|Display", Meta = (ClampMin = "1.0", ClampMax = "1000.0"))
	float centerSphereRadius = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid Preview|Display")
	bool bHighlightPentagons = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid Preview|Display")
	bool bShowNeighborConnections = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid Preview|Display")
	bool bShowCellIDs = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid Preview|Display")
	bool bColorByIcosahedronFaces = false;

	// === COLORS ===

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid Preview|Colors")
	FLinearColor hexagonColor = FLinearColor::Green;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid Preview|Colors")
	FLinearColor pentagonColor = FLinearColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid Preview|Colors")
	FLinearColor neighborConnectionColor = FLinearColor(0.5f, 0.5f, 1.0f, 0.3f);

	// === FILTERING ===

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid Preview|Filtering")
	bool bUseCellIDRange = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid Preview|Filtering", Meta = (EditCondition = "bUseCellIDRange", ClampMin = "0"))
	int32 minCellID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid Preview|Filtering", Meta = (EditCondition = "bUseCellIDRange", ClampMin = "0"))
	int32 maxCellID = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid Preview|Filtering")
	bool bFilterByIcosahedronFace = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid Preview|Filtering", Meta = (EditCondition = "bFilterByIcosahedronFace", ClampMin = "0", ClampMax = "19"))
	int32 icosahedronFaceFilter = 0;

	// === SELECTION ===

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid Preview|Selection")
	int32 selectedCellID = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid Preview|Selection")
	bool bHighlightSelected = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid Preview|Selection")
	FLinearColor selectedCellColor = FLinearColor::Yellow;

	// === STATISTICS DISPLAY ===

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hex Grid Preview|Statistics")
	bool bShowStatistics = true;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }
#endif

	// === PUBLIC API ===

	UFUNCTION(BlueprintCallable, Category = "Hex Grid Preview")
	void GenerateGrid(int32 level);

	UFUNCTION(BlueprintCallable, Category = "Hex Grid Preview")
	void SelectCell(int32 cellId);

	UFUNCTION(BlueprintCallable, Category = "Hex Grid Preview")
	void SelectCellAtPosition(FVector worldPosition);

	UFUNCTION(BlueprintCallable, Category = "Hex Grid Preview")
	void ClearSelection();

	UFUNCTION(BlueprintCallable, Category = "Hex Grid Preview")
	FString GetSelectedCellInfo() const;

private:
	
	// === DRAWING FUNCTIONS ===

	void DrawGrid();

	void DrawCell(const FHexCell& cell, bool bIsSelected = false);

	void DrawCellOutline(const FHexCell& cell, FLinearColor color);

	void DrawCellCenter(const FHexCell& cell, FLinearColor color);

	void DrawNeighborConnections(const FHexCell& cell);

	void DrawCellID(const FHexCell& cell);

	void DrawStatistics();

	FLinearColor GetCellColor(const FHexCell& cell) const;

	bool ShouldDrawCell(const FHexCell& cell) const;

	FVector GridToWorldPosition(const FVector& gridPosition) const;

	FLinearColor GetFaceColor(uint8 faceIndex) const;
};
