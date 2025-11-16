// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0.
// Noncommercial use only. Commercial use requires written permission.
// See https://polyformproject.org/licenses/noncommercial/1.0.0/
#include "HexGridEditorUtility.h"

#include "HexGridGenerator.h"
#include "HexGridViewActor.h"
#include "AssetRegistry/AssetRegistryModule.h"

UHexGridAsset* UHexGridEditorUtility::GenerateHexGridPreview(int32 level, TArray<FString>& outErrors)
{
	return UHexGridGenerator::GenerateHexGrid(level, outErrors);
}

UHexGridAsset* UHexGridEditorUtility::GenerateAndSaveHexGrid(
	int32 level,
	const FString& assetName,
	const FString& assetPath,
	TArray<FString>& outErrors)
{
	FString cleanPath = assetPath;
	if (!cleanPath.EndsWith(TEXT("/")))
	{
		cleanPath += TEXT("/");
	}

	return UHexGridGenerator::CreateHexGridAsset(level, assetName, cleanPath, outErrors);
}

int32 UHexGridEditorUtility::GetExpectedCellCount(int32 level)
{
	return UHexGridAsset::GetExpectedCellCount(level);
}

FString UHexGridEditorUtility::GetLevelRecommendation(int32 level)
{
	switch (level)
	{
	case 0:
	case 1:
	case 2:
		return TEXT("Very Low - Testing and Prototyping (~10-300 cells)");

	case 3:
		return TEXT("Low - Small Asteroids (~1,280 cells)");

	case 4:
		return TEXT("Medium - Small Moons (~5,120 cells)");

	case 5:
		return TEXT("High - Medium Moons/Small Planets (~20,480 cells)");

	case 6:
		return TEXT("Very High - Large Planets (~81,920 cells)");

	case 7:
		return TEXT("Extreme - Very Large Planets (~327,680 cells)");

	case 8:
		return TEXT("Insane - Huge Planets (~1,310,720 cells) - WARNING: Heavy");

	case 9:
		return TEXT("Ludicrous - Massive Planets (~5,242,880 cells) - WARNING: VERY Heavy, NOT RECOMMENDED");

	case 10:
		return TEXT("Ridiculous - Gargantuan Planets (~20,971,520 cells) - WARNING: EXTREMELY Heavy, NOT RECOMMENDED, WILL HANG");

	default:
		if (level > 10)
		{
			return TEXT("Beyond Ridiculous - Unthinkable Sizes - WARNING: UNUSABLE, WILL CRASH");
		}

		return TEXT("Invalid Level");
	}
}

bool UHexGridEditorUtility::ValidateHexGrid(UHexGridAsset* gridAsset, TArray<FString>& outErrors)
{
	if (!gridAsset)
	{
		outErrors.Add(TEXT("Invalid HexGridAsset."));
		return false;
	}

	return gridAsset->ValidateGrid(outErrors);
}

class AHexGridViewActor* UHexGridEditorUtility::SpawnPreviewActor(
	UObject* worldContextObject,
	UHexGridAsset* gridAsset,
	FVector location,
	float radius)
{
	if (!worldContextObject)
	{
		return nullptr;
	}

	UWorld* world = worldContextObject->GetWorld();
	if (!world)
	{
		return nullptr;
	}

	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AHexGridViewActor* previewActor = world->SpawnActor<AHexGridViewActor>(
		location, FRotator::ZeroRotator, spawnParams);

	if (previewActor)
	{
		previewActor->GridAsset = gridAsset;
		previewActor->displayRadius = radius;

#if WITH_EDITOR
		previewActor->SetActorLabel(FString::Printf(TEXT("HexGrid_Preview_L%d"), gridAsset ? gridAsset->GridLevel : 0));
#endif
	}

	return previewActor;
}

TArray<UHexGridAsset*> UHexGridEditorUtility::FindAllHexGridAssets()
{
	TArray<UHexGridAsset*> foundAssets;

#if WITH_EDITOR
	FAssetRegistryModule& assetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& assetRegistry = assetRegistryModule.Get();

	TArray<FAssetData> assetDataList;
	assetRegistry.GetAssetsByClass(UHexGridAsset::StaticClass()->GetClassPathName(), assetDataList, true);

	for (const FAssetData& assetData : assetDataList)
	{
		UHexGridAsset* gridAsset = Cast<UHexGridAsset>(assetData.GetAsset());
		if (gridAsset)
		{
			foundAssets.Add(gridAsset);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Found %d HexGridAsset(s) in the project."), foundAssets.Num());
#endif

	return foundAssets;
}

FString UHexGridEditorUtility::GetGridStatistics(UHexGridAsset* gridAsset)
{
	if (!gridAsset)
	{
		return TEXT("No grid asset provided.");
	}

	FString stats = FString::Printf(
		TEXT("Grid Level: %d\n")
		TEXT("Total Cells: %d\n")
		TEXT("   - Hexagons: %d\n")
		TEXT("   - Pentagons: %d\n")
		TEXT("Cell Area Statistics (unit sphere):\n")
		TEXT("   - Min Area: %.6f\n")
		TEXT("   - Max Area: %.6f\n")
		TEXT("   - Avg Area: %.6f\n")
		TEXT("   - Std Dev : %.6f\n")
		TEXT("Uniformity: %.2f%% (100%% = perfectly uniform)"),
		gridAsset->GridLevel,
		gridAsset->TotalCellCount,
		gridAsset->HexagonCount,
		gridAsset->PentagonCount,
		gridAsset->MinCellArea,
		gridAsset->MaxCellArea,
		gridAsset->AverageCellArea,
		gridAsset->AreaStandardDeviation,
		(1.0f - FMath::Min(gridAsset->AreaStandardDeviation / gridAsset->AverageCellArea, 1.0f)) * 100.0f
	);

	return stats;
}

void UHexGridEditorUtility::BatchGenerateGrids(
	int32 minLevel,
	int32 maxLevel,
	const FString& assetPath,
	TArray<UHexGridAsset*>& outGeneratedAssets,
	TArray<FString>& outErrors)
{
	outGeneratedAssets.Empty();
	outErrors.Empty();

	if (minLevel < 0 || maxLevel > 10 || minLevel > maxLevel)
	{
		outErrors.Add(TEXT("Invalid level range specified for batch generation (must be 0-10, min <= max)."));
		return;
	}

	FString cleanPath = assetPath;
	if (!cleanPath.EndsWith(TEXT("/")))
	{
		cleanPath += TEXT("/");
	}

	UE_LOG(LogTemp, Log, TEXT("Starting batch generation of hex grids from level %d to %d..."), minLevel, maxLevel);

	for (int32 level = minLevel; level <= maxLevel; ++level)
	{
		FString assetName = FString::Printf(TEXT("HexGrid_L%d"), level);
		TArray<FString> levelErrors;

		UE_LOG(LogTemp, Log, TEXT("    Generating HexGrid level %d..."), level);

		UHexGridAsset* grid = UHexGridGenerator::CreateHexGridAsset(level, assetName, cleanPath, levelErrors);
		if (grid)
		{
			outGeneratedAssets.Add(grid);
			UE_LOG(LogTemp, Log, TEXT("    Successfully generated HexGrid level %d, %d cells."), level, grid->TotalCellCount);
		}
		else
		{
			FString errorMsg = FString::Printf(TEXT("    Failed to generate HexGrid level %d:"), level);
			outErrors.Add(errorMsg);
			outErrors.Append(levelErrors);
			UE_LOG(LogTemp, Error, TEXT("    %s"), *errorMsg);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Batch generation completed: %d/%d successful."),
		outGeneratedAssets.Num(), maxLevel - minLevel + 1);
}

