// Fill out your copyright notice in the Description page of Project Settings.
//Unione unit

#include "Sniper.h"
#include "UObject/ConstructorHelpers.h"
#include "Brawler.h" 
#include "Kismet/KismetMathLibrary.h" 

ASniper::ASniper()
{
 
    MovementRange = 3;
    AttackType = "Ranged";
    AttackRange = 10;
    MinDamage = 4;
    MaxDamage = 8;
    MaxHealth = 20;
    Health = MaxHealth;

   
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> BlueMatFinder(TEXT("/Game/texture/Sniper1_blue.Sniper1_blue"));
    if (BlueMatFinder.Succeeded())
    {
        BlueMaterial = BlueMatFinder.Object;
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> RedMatFinder(TEXT("/Game/texture/Sniper1_red.Sniper1_red"));
    if (RedMatFinder.Succeeded())
    {
        RedMaterial = RedMatFinder.Object;
    }
}
void ASniper::BeginPlay()
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

UMaterialInterface* ASniper::GetDisplayMaterial() const
{
    return bIsPlayerTeam ? BlueMaterial : RedMaterial;
}

void ASniper::Attack(AUnitPawn* TargetUnit)
{
    if (TargetUnit)
    {
        int32 Distance = FMath::Abs( CurrentX - TargetUnit->CurrentX) +
            FMath::Abs( CurrentY - TargetUnit->CurrentY);

        
        if (Distance <= AttackRange)
        {
            int32 Damage = FMath::RandRange(MinDamage, MaxDamage);
            TargetUnit->Health -= Damage;
            if (TargetUnit->Health <= 0)
            {
                TargetUnit->Destroy();
            }

            bool bCounterAttack = false;
            if (TargetUnit->IsA(ASniper::StaticClass()))
            {
                bCounterAttack = true;
            }
            else if (TargetUnit->IsA(ABrawler::StaticClass()) && Distance == 1)
            {
                bCounterAttack = true;
            }

            if (bCounterAttack)
            {
                int32 CounterDamage = FMath::RandRange(1, 3);
                Health -= CounterDamage;
                if (Health <= 0)
                {
                    this->Destroy();
                }
            }
        }
    }
}