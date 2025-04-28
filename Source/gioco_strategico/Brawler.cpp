// Fill out your copyright notice in the Description page of Project Settings.

#include "Brawler.h"
#include "UObject/ConstructorHelpers.h"
ABrawler::ABrawler()
{

    MovementRange = 6;
    AttackType = "Melee";
    AttackRange = 1;
    MinDamage = 1;
    MaxDamage = 6;
    MaxHealth = 40;
    Health = MaxHealth;

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> BlueMatFinder(TEXT("/Game/texture/Brawler1_blue.Brawler1_blue"));
    if (BlueMatFinder.Succeeded())
    {
        BlueMaterial = BlueMatFinder.Object;
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> RedMatFinder(TEXT("/Game/texture/Brawler1_red.Brawler1_red"));
    if (RedMatFinder.Succeeded())
    {
        RedMaterial = RedMatFinder.Object;
    }
}

void ABrawler::BeginPlay()
{
    Super::BeginPlay();

    if (bIsPlayerTeam)
    {
        DisplayMaterial = BlueMaterial;  
        UnitMesh->SetMaterial(0, BlueMaterial);
    }
    else
    {
        DisplayMaterial = RedMaterial; 
        UnitMesh->SetMaterial(0, RedMaterial);
    }

}
UMaterialInterface* ABrawler::GetDisplayMaterial() const
{
    return bIsPlayerTeam ? BlueMaterial : RedMaterial;
}
