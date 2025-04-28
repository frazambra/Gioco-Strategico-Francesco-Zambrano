// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnitPawn.h"
#include "Brawler.generated.h"

/**
 * 
 */
UCLASS()
class GIOCO_STRATEGICO_API ABrawler : public AUnitPawn
{
	GENERATED_BODY()
	
public:
	ABrawler();

    virtual void BeginPlay() override;

   
    virtual UMaterialInterface* GetDisplayMaterial() const;

private:
  
    UPROPERTY(EditDefaultsOnly, Category = "Appearance")
    UMaterialInterface* BlueMaterial;

    UPROPERTY(EditDefaultsOnly, Category = "Appearance")
    UMaterialInterface* RedMaterial;
};
