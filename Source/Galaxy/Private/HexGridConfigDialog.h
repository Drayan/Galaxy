// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0.
// Noncommercial use only. Commercial use requires written permission.
// See https://polyformproject.org/licenses/noncommercial/1.0.0/


#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class GALAXY_API SHexGridConfigDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SHexGridConfigDialog) : _DefaultSubdivision(4)
	{}
		SLATE_ARGUMENT(int32, DefaultSubdivision)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	static bool ShowDialog(int32& outSubdivisionLevel);
	
	int32 GetSubdisivionLevel() const { return SelectedSubdivision; }

	bool WasConfirmed() const { return bConfirmed; }

private:

	FReply OnOKClicked();

	FReply OnCancelClicked();

	void OnSubdivisionChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);

	FText GetEstimatedTileCountText() const;

	FText GetRecommendedUseText() const;

	TArray<TSharedPtr<FString>> SubdivisionOptions;

	int32 SelectedSubdivision;

	bool bConfirmed;

	TWeakPtr<SWindow> DialogWindow;
};
