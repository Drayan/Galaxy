// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0.
// Noncommercial use only. Commercial use requires written permission.
// See https://polyformproject.org/licenses/noncommercial/1.0.0/
#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BiomeData.generated.h"

UCLASS(BlueprintType)
class GALAXY_API UBiomeData : public UDataAsset
{
	GENERATED_BODY()

public:
	// === Identity ===

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FName BiomeID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FText BiomeName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
	FText BiomeDescription;

	// === Visuals ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FLinearColor MapColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	TObjectPtr<UTexture2D> Icon;

	// === Terrain Generation ===

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain")

	// === Gameplay ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
	float MovementCostMultiplier = 1.0f;
};

