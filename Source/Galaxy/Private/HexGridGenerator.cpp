// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0.
// Noncommercial use only. Commercial use requires written permission.
// See https://polyformproject.org/licenses/noncommercial/1.0.0/
#include "HexGridGenerator.h"

UHexGridAsset* UHexGridGenerator::GenerateHexGrid(int32 level, TArray<FString>& OutErrors)
{
	OutErrors.Empty();

	if (level < 0 || level > 10)
	{
		OutErrors.Add(FString::Printf(TEXT("Invalid level %d. Level must be between 0 and 10."), level));
		return nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("HexGridGenerator: Starting generation for level %d."), level);

	return nullptr;
}
