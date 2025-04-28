// Fill out your copyright notice in the Description page of Project Settings.


#include "MyInterface.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UMyInterface::UpdateTurnText(FText Turno)
{
	if (TurnText)
	{
		TurnText->SetText(Turno);
	}
}
	void UMyInterface::AddHistoryEntry(FText NewEntry)
	{
		if (HistoryText)
		{

			if (HistoryText->Visibility == ESlateVisibility::Collapsed || HistoryText->Visibility == ESlateVisibility::Hidden)
			{
				HistoryText->SetVisibility(ESlateVisibility::Visible);
			}
			FString CurrentHistory = HistoryText->GetText().ToString();
            FString UpdatedHistory;
            if (!CurrentHistory.IsEmpty())
            {
                UpdatedHistory = NewEntry.ToString() + "\n" + CurrentHistory;
            }
            else
            {
                UpdatedHistory = NewEntry.ToString();
            }
            HistoryText->SetText(FText::FromString(UpdatedHistory));
        }
	}

    void UMyInterface::UpdateHealthBar_Player1(float CurrentHealth, float MaxHealth)
    {
        if (HealthBar_Player1)
        {
            float Percent = (MaxHealth > 0.f) ? CurrentHealth / MaxHealth : 0.f;
            HealthBar_Player1->SetPercent(Percent);
            HealthBar_Player1->SetVisibility(ESlateVisibility::Visible);
        }
        if (HealthText_Player1)
        {
            FString HealthStr = FString::Printf(TEXT("Sniper : %.0f/%.0f"), CurrentHealth, MaxHealth);
            HealthText_Player1->SetText(FText::FromString(HealthStr));
            HealthText_Player1->SetVisibility(ESlateVisibility::Visible);
        }
    }

    void UMyInterface::UpdateHealthBar_Player2(float CurrentHealth, float MaxHealth)
    {
        if (HealthBar_Player2)
        {
            float Percent = (MaxHealth > 0.f) ? CurrentHealth / MaxHealth : 0.f;
            HealthBar_Player2->SetPercent(Percent);
            HealthBar_Player2->SetVisibility(ESlateVisibility::Visible);
        }
        if (HealthText_Player2)
        {
            FString HealthStr = FString::Printf(TEXT("Brawler : %.0f/%.0f"), CurrentHealth, MaxHealth);
            HealthText_Player2->SetText(FText::FromString(HealthStr));
            HealthText_Player2->SetVisibility(ESlateVisibility::Visible);
        }
    }

    void UMyInterface::UpdateHealthBar_AI1(float CurrentHealth, float MaxHealth)
    {
        if (HealthBar_AI1)
        {
            float Percent = (MaxHealth > 0.f) ? CurrentHealth / MaxHealth : 0.f;
            HealthBar_AI1->SetPercent(Percent);
            HealthBar_AI1->SetVisibility(ESlateVisibility::Visible);
        }
        if (HealthText_AI1)
        {
            FString HealthStr = FString::Printf(TEXT("Sniper : %.0f/%.0f"), CurrentHealth, MaxHealth);
            HealthText_AI1->SetText(FText::FromString(HealthStr));
            HealthText_AI1->SetVisibility(ESlateVisibility::Visible);
        }
    }

    void UMyInterface::UpdateHealthBar_AI2(float CurrentHealth, float MaxHealth)
    {
        if (HealthBar_AI2)
        {
            float Percent = (MaxHealth > 0.f) ? CurrentHealth / MaxHealth : 0.f;
            HealthBar_AI2->SetPercent(Percent);
            HealthBar_AI2->SetVisibility(ESlateVisibility::Visible);
        }
        if (HealthText_AI2)
        {
            FString HealthStr = FString::Printf(TEXT("Brawler : %.0f/%.0f"), CurrentHealth, MaxHealth);
            HealthText_AI2->SetText(FText::FromString(HealthStr));
            HealthText_AI2->SetVisibility(ESlateVisibility::Visible);
        }
    }

    void UMyInterface::ShowRules(FText Rules)
    {
        if (RulesText)
        {
            RulesText->SetText(Rules);
            RulesText->SetVisibility(ESlateVisibility::Visible);
        }
    }