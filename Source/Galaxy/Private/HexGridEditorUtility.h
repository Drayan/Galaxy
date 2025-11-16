// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0.
// Noncommercial use only. Commercial use requires written permission.
// See https://polyformproject.org/licenses/noncommercial/1.0.0/
#pragma once

#include "CoreMinimal.h"
#include "HexGridAsset.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HexGridEditorUtility.generated.h"

/// <summary>
/// Blueprint function library for hex grid editor utilities
/// Use these functions in Editor Utility Widgets or Blueprints
/// </summary>
UCLASS()
class GALAXY_API UHexGridEditorUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Hex Grid|Editor", meta = (DevelopmentOnly))
	static UHexGridAsset* GenerateHexGridPreview(int32 level, TArray<FString>& outErrors);

	UFUNCTION(BlueprintCallable, Category = "Hex Grid|Editor", meta = (DevelopmentOnly))
	static UHexGridAsset* GenerateAndSaveHexGrid(
		int32 level,
		const FString& assetName,
		const FString& assetPath,
		TArray<FString>& outErrors);

	UFUNCTION(BlueprintCallable, Category = "Hex Grid|Editor", meta = (DevelopmentOnly))
	static int32 GetExpectedCellCount(int32 level);

	UFUNCTION(BlueprintCallable, Category = "Hex Grid|Editor", meta = (DevelopmentOnly))
	static FString GetLevelRecommendation(int32 level);

	UFUNCTION(BlueprintCallable, Category = "Hex Grid|Editor", meta = (DevelopmentOnly))
	static bool ValidateHexGrid(UHexGridAsset* gridAsset, TArray<FString>& outErrors);

	UFUNCTION(BlueprintCallable, Category = "Hex Grid|Editor", meta = (DevelopmentOnly))
	static class AHexGridViewActor* SpawnPreviewActor(
		UObject* worldContextObject,
		UHexGridAsset* gridAsset,
		FVector location,
		float radius);

	UFUNCTION(BlueprintCallable, Category = "Hex Grid|Editor", meta = (DevelopmentOnly))
	static TArray<UHexGridAsset*> FindAllHexGridAssets();

	UFUNCTION(BlueprintCallable, Category = "Hex Grid|Editor", meta = (DevelopmentOnly))
	static FString GetGridStatistics(UHexGridAsset* gridAsset);

	UFUNCTION(BlueprintCallable, Category = "Hex Grid|Editor", meta = (DevelopmentOnly))
	static void BatchGenerateGrids(
		int32 minLevel,
		int32 maxLevel,
		const FString& assetPath,
		TArray<UHexGridAsset*>& outGeneratedAssets,
		TArray<FString>& outErrors);
};
