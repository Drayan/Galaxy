// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0.
// Noncommercial use only. Commercial use requires written permission.
// See https://polyformproject.org/licenses/noncommercial/1.0.0/
#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "HexGridAssetFactory.generated.h"

/// <summary>
/// Factory for creating HexGridAsset in the Content Browser.
/// </summary>
UCLASS()
class GALAXY_API UHexGridAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UHexGridAssetFactory();

	virtual UObject* FactoryCreateNew(
		UClass* InClass,
		UObject* InParent,
		FName InName,
		EObjectFlags Flags,
		UObject* Context,
		FFeedbackContext* Warn) override;
	virtual bool ConfigureProperties() override;
	virtual bool ShouldShowInNewMenu() const override;
	virtual FText GetDisplayName() const override;

	UPROPERTY()
	int32 SubdivisionLevel = 4;
};
