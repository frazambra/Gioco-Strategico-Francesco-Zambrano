// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "UnitPawn.h"
#include "Sniper.generated.h"


UCLASS()
class GIOCO_STRATEGICO_API ASniper : public AUnitPawn
{
	GENERATED_BODY()
	
public:
	ASniper();

	virtual void Attack(AUnitPawn* TargetUnit) override;

	virtual UMaterialInterface* GetDisplayMaterial() const override;

	virtual void BeginPlay() override; 


protected:
	
	
	UPROPERTY(EditDefaultsOnly, Category = "Appearance")
	UMaterialInterface* BlueMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Appearance")
	UMaterialInterface* RedMaterial;
};
