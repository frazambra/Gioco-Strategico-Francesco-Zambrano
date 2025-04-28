// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyInterface.generated.h"

/**
 * 
 */
UCLASS()
class GIOCO_STRATEGICO_API UMyInterface : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UTextBlock* TurnText;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UTextBlock* HistoryText;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UTextBlock* RulesText;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UProgressBar* HealthBar_Player1;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UTextBlock* HealthText_Player1;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UProgressBar* HealthBar_Player2;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UTextBlock* HealthText_Player2;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UProgressBar* HealthBar_AI1;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UTextBlock* HealthText_AI1;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UProgressBar* HealthBar_AI2;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UTextBlock* HealthText_AI2;

	void UpdateTurnText(FText Turno);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void AddHistoryEntry(FText NewEntry);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHealthBar_Player1(float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHealthBar_Player2(float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHealthBar_AI1(float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHealthBar_AI2(float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowRules(FText Rules);
};
