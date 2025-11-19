// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0.
// Noncommercial use only. Commercial use requires written permission.
// See https://polyformproject.org/licenses/noncommercial/1.0.0/
#include "HexGridAssetFactory.h"
#include "HexGridAsset.h"
#include "HexGridConfigDialog.h"
#include "HexGridEditorUtility.h"
#include "HexGridGenerator.h"


UHexGridAssetFactory::UHexGridAssetFactory()
{
	bCreateNew = true;
	bEditAfterNew = false;
	SupportedClass = UHexGridAsset::StaticClass();
}

UObject* UHexGridAssetFactory::FactoryCreateNew(
	UClass* InClass,
	UObject* InParent,
	FName InName,
	EObjectFlags Flags,
	UObject* Context,
	FFeedbackContext* Warn)
{
	UHexGridAsset* newAsset = NewObject<UHexGridAsset>(InParent, InClass, InName, Flags);
	if (newAsset)
	{
		TArray<FString> Errors;
		bool bSuccess = UHexGridGenerator::PopulateHexGridAsset(newAsset, SubdivisionLevel, Errors);

		if (!bSuccess)
		{
			// Log errors
			if (Warn)
			{
				for (const FString& Error : Errors)
				{
					Warn->Logf(ELogVerbosity::Error, TEXT("%s"), *Error);
				}
			}

			// Show error dialog
			FText ErrorTitle = FText::FromString(TEXT("Hex Grid Generation Failed"));
			FText ErrorMessage = FText::FromString(FString::Printf(TEXT("Failed to create Hex Grid Asset due to the following errors:\n\n%s"),
				*FString::Join(Errors, TEXT("\n"))));
			FMessageDialog::Open(EAppMsgType::Ok, ErrorMessage, ErrorTitle);

			return nullptr;
		}

		UE_LOG(
			LogTemp,
			Log,
			TEXT("HexGridAssetFactory: Created new HexGridAsset '%s' with subdivision level %d."),
			*InName.ToString(),
			SubdivisionLevel);
	}
	
	return newAsset;
}

bool UHexGridAssetFactory::ConfigureProperties()
{
	return SHexGridConfigDialog::ShowDialog(SubdivisionLevel);
}

bool UHexGridAssetFactory::ShouldShowInNewMenu() const
{
	return Super::ShouldShowInNewMenu();
}

FText UHexGridAssetFactory::GetDisplayName() const
{
	return FText::FromString(TEXT("Hex Grid"));
}
