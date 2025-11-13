// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0.
// Noncommercial use only. Commercial use requires written permission.
// See https://polyformproject.org/licenses/noncommercial/1.0.0/
#pragma once

#include "CoreMinimal.h"
#include "HexGridGenerator.generated.h"

UCLASS()
class GALAXY_API UHexGridGenerator : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Hex Grid Generation")
	static UHexGridAsset* GenerateHexGrid(int32 level, TArray<FString>& OutErrors);
};
