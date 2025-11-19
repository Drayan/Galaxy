// (c) 2025 Michaël Desmedt. Licensed under the PolyForm Noncommercial License 1.0.0.
// Noncommercial use only. Commercial use requires written permission.
// See https://polyformproject.org/licenses/noncommercial/1.0.0/


#include "HexGridConfigDialog.h"

#include "SlateOptMacros.h"
#include "Widgets/Layout/SUniformGridPanel.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SHexGridConfigDialog::Construct(const FArguments& InArgs)
{
	SelectedSubdivision = InArgs._DefaultSubdivision;
	bConfirmed = false;

	// Populate subdivision options
	for (int32 i = 3; i <= 8; ++i)
	{
		SubdivisionOptions.Add(MakeShared<FString>(FString::Printf(TEXT("Level %d"), i)));
	}

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(16.0f)
		[
			SNew(SVerticalBox)

			// Title
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 16)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Configure Hex Grid")))
				.Font(FAppStyle::GetFontStyle("HeadingLarge"))
			]

			// Subdivision selection
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 8)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0, 0, 16, 0)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Subdivision Level:")))
					.MinDesiredWidth(120.0f)
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&SubdivisionOptions)
					.OnGenerateWidget_Lambda(
						[](TSharedPtr<FString> InItem)
						{
							return SNew(STextBlock)
								.Text(FText::FromString(*InItem));
						})
					.OnSelectionChanged(this, &SHexGridConfigDialog::OnSubdivisionChanged)
					.InitiallySelectedItem(SubdivisionOptions[SelectedSubdivision - 3])
					[
						SNew(STextBlock)
						.Text_Lambda(
							[this]()
							{
								return FText::FromString(FString::Printf(TEXT("Level %d"), SelectedSubdivision));
							})
					]
				]
			]

			// Info section
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 16, 0, 8)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("DetailsView.CategoryTop"))
				.Padding(8.0f)
				[
					SNew(SVerticalBox)

					// Estimated tile count
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 4)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Estimated Tiles:")))
							.Font(FAppStyle::GetFontStyle("BoldFont"))
						]

						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.Padding(8, 0, 0, 0)
						[
							SNew(STextBlock)
							.Text(this, &SHexGridConfigDialog::GetEstimatedTileCountText)
						]
					]

					// Recommended use
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 4)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Recommended For:")))
							.Font(FAppStyle::GetFontStyle("BoldFont"))
						]

						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.Padding(8, 0, 0, 0)
						[
							SNew(STextBlock)
							.Text(this, &SHexGridConfigDialog::GetRecommendedUseText)
						]
					]
				]
			]

			// Buttons
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 16, 0, 0)
			.HAlign(HAlign_Right)
			[
				SNew(SUniformGridPanel)
				.SlotPadding(FAppStyle::GetMargin("StandardDialog.SlotPadding"))

				+ SUniformGridPanel::Slot(0, 0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Create")))
					.HAlign(HAlign_Center)
					.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
					.OnClicked(this, &SHexGridConfigDialog::OnOKClicked)
					.ButtonStyle(FAppStyle::Get(), "PrimaryButton")
				]

				+ SUniformGridPanel::Slot(1, 0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Cancel")))
					.HAlign(HAlign_Center)
					.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
					.OnClicked(this, &SHexGridConfigDialog::OnCancelClicked)
				]
			]
		]
	];
}

bool SHexGridConfigDialog::ShowDialog(int32& outSubdivisionLevel)
{
	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(FText::FromString(TEXT("Create Hex Grid Asset")))
		.SizingRule(ESizingRule::Autosized)
		.SupportsMinimize(false)
		.SupportsMaximize(false)
		.IsTopmostWindow(true);

	TSharedRef<SHexGridConfigDialog> Dialog = SNew(SHexGridConfigDialog)
		.DefaultSubdivision(outSubdivisionLevel);

	Window->SetContent(Dialog);

	Dialog->DialogWindow = Window;

	FSlateApplication::Get().AddModalWindow(Window, FSlateApplication::Get().GetActiveTopLevelWindow());

	if (Dialog->WasConfirmed())
	{
		outSubdivisionLevel = Dialog->GetSubdisivionLevel();
		return true;
	}

	return false;
}

FReply SHexGridConfigDialog::OnOKClicked()
{
	bConfirmed = true;
	if (DialogWindow.IsValid())
	{
		DialogWindow.Pin()->RequestDestroyWindow();
	}

	return FReply::Handled();
}

FReply SHexGridConfigDialog::OnCancelClicked()
{
	bConfirmed = false;
	if (DialogWindow.IsValid())
	{
		DialogWindow.Pin()->RequestDestroyWindow();
	}

	return FReply::Handled();
}

void SHexGridConfigDialog::OnSubdivisionChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo)
{
	if (NewValue.IsValid())
	{
		FString LevelStr = *NewValue;
		LevelStr.RemoveFromStart(TEXT("Level "));
		SelectedSubdivision = FCString::Atoi(*LevelStr);
	}
}

FText SHexGridConfigDialog::GetEstimatedTileCountText() const
{
	int32 EstimatedTiles = 10 * FMath::Pow(4.0f, SelectedSubdivision) + 2;

	return FText::FromString(FString::Printf(TEXT("%s tiles"),
		*FText::AsNumber(EstimatedTiles, &FNumberFormattingOptions::DefaultNoGrouping()).ToString()));
}

FText SHexGridConfigDialog::GetRecommendedUseText() const
{
	switch (SelectedSubdivision)
	{
	case 3:
		return FText::FromString(TEXT("Asteroids, prototyping (~642 tiles)"));

	case 4:
		return FText::FromString(TEXT("Big asteroids, small moons, testing (~2,562 tiles)"));

	case 5:
		return FText::FromString(TEXT("Large moons, small planets(~10,242 tiles)"));

	case 6:
		return FText::FromString(TEXT("Planets (~40,962 tiles)"));

	case 7:
		return FText::FromString(TEXT("Gas giants (~163,842 tiles)"));

	case 8:
		return FText::FromString(TEXT("Stars (655,362 tiles)"));
	default:
		return FText::FromString(TEXT("Custom configuration"));
	}
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
