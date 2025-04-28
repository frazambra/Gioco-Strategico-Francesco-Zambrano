// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitPawn.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AUnitPawn::AUnitPawn()
{
    PrimaryActorTick.bCanEverTick = false;

    UnitMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PawnMesh"));
    RootComponent = UnitMesh;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Plane.Plane'"));
    if (MeshFinder.Succeeded())
    {
        UnitMesh->SetStaticMesh(MeshFinder.Object);
    }

    UnitMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    UnitMesh->SetCollisionObjectType(ECC_WorldDynamic);
    UnitMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    UnitMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    MovementRange = 0;
    AttackType = "None";
    AttackRange = 0;
    MinDamage = 0;
    MaxDamage = 0;
    Health = 0;
    bIsPlayerTeam = false;
    bHasMovedThisTurn = false;
    bHasAttackedThisTurn = false;
    bHasActedThisTurn = false;
    CurrentX = 0;
    CurrentY = 0;
}

void AUnitPawn::BeginPlay()
{
    Super::BeginPlay();
    if (DisplayMaterial)
    {
        UnitMesh->SetMaterial(0, DisplayMaterial);
    }
    UnitMesh->OnClicked.AddDynamic(this, &AUnitPawn::HandleMeshClicked);
}

void AUnitPawn::HandleMeshClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
    OnUnitClicked.Broadcast(this);
}
void AUnitPawn::Move(int32 NewX, int32 NewY)
{
    CurrentX = NewX;
    CurrentY = NewY;
    SetActorLocation(FVector(NewX * 100, NewY * 100, GetActorLocation().Z));
}

void AUnitPawn::Attack(AUnitPawn* TargetUnit)
{
    if (TargetUnit)
    {
        int32 Distance = FMath::Abs(CurrentX - TargetUnit->CurrentX) + FMath::Abs(CurrentY - TargetUnit->CurrentY);
        if (Distance <= AttackRange)
        {
            int32 Damage = FMath::RandRange(MinDamage, MaxDamage);
            TargetUnit->Health -= Damage;
            if (TargetUnit->Health <= 0)
            {
                TargetUnit->Destroy();
            }
        }
    }
}
