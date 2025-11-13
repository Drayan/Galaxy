// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0.
// Noncommercial use only. Commercial use requires written permission.
// See https://polyformproject.org/licenses/noncommercial/1.0.0/
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HexCell.h"
#include "HexGridAsset.generated.h"

UCLASS(BlueprintType)
class GALAXY_API UHexGridAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Infos")
	int32 GridLevel = 3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Infos")
	int32 TotalCellCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Infos")
	int32 HexagonCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Infos")
	int32 PentagonCount = 12;

	UPROPERTY(VisibleAnywhere, Category = "Grid Data")
	TArray<FHexCell> Cells;

	UPROPERTY(VisibleAnywhere, Category = "Grid Data")
	TArray<int32> PentagonCellsIds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Stats")
	float MinCellArea = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Stats")
	float MaxCellArea = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Stats")
	float AverageCellArea = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Stats")
	float AreaStandardDeviation = 0.0f;

	UFUNCTION(BlueprintCallable, Category = "Hex Grid")
	const FHexCell& GetCellById(int32 CellId) const;

	UFUNCTION(BlueprintCallable, Category = "Hex Grid")
	void GetNeighbors(int32 CellId, TArray<int32>& outNeighborIds) const;

	UFUNCTION(BlueprintCallable, Category = "Hex Grid")
	int32 FindCellAtPosition(const FVector& Position) const;

	UFUNCTION(BlueprintCallable, Category = "Hex Grid")
	void FindClosestCells(const FVector& Position, TArray<int32>& outCells, int32 Count = 3) const;

	UFUNCTION(BlueprintCallable, Category = "Hex Grid")
	void FindCellsInRadius(FVector position, int32 Radius, TArray<int32>& outCells) const;

	UFUNCTION(BlueprintCallable, Category = "Hex Grid")
	const TArray<int32>& GetPentagons() const;

	UFUNCTION(BlueprintCallable, Category = "Hex Grid")
	bool ValidateGrid(TArray<FString>& outErrors) const;

	void CalculateStatistics();

	static int32 GetExpectedCellCount(int32 Level);
};
