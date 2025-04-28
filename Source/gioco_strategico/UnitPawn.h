// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnitPawn.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitClicked, class AUnitPawn*, ClickedUnit);

UCLASS()
class GIOCO_STRATEGICO_API AUnitPawn : public AActor
{
	GENERATED_BODY()
	
public:	
	AUnitPawn();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* UnitMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
	UMaterialInterface* DisplayMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    int32 MovementRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    FString AttackType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    int32 AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    int32 MinDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    int32 MaxDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    int32 Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    bool bIsPlayerTeam;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    bool bHasActedThisTurn;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Unit")
    bool bHasMovedThisTurn;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Unit")
    bool bHasAttackedThisTurn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    int32 MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit")
    int32 CurrentX;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit")
    int32 CurrentY;

    UPROPERTY(BlueprintAssignable, Category = "Unit")
    FOnUnitClicked OnUnitClicked;

    UFUNCTION(BlueprintCallable, Category = "Unit")
    void Move(int32 NewX, int32 NewY);

    UFUNCTION(BlueprintCallable, Category = "Unit")
    virtual void Attack(AUnitPawn* TargetUnit);

    UFUNCTION(BlueprintCallable, Category = "Unit")
    virtual UMaterialInterface* GetDisplayMaterial() const
    {
        return UnitMesh->GetMaterial(0);
    }


protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleMeshClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);
};
