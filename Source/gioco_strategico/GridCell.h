// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridCell.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGridCellClicked, AGridCell*, ClickedCell);

UCLASS()
class GIOCO_STRATEGICO_API AGridCell : public AActor
{
	GENERATED_BODY()
	
public:	

	AGridCell();

protected:

	virtual void BeginPlay() override;
	virtual void NotifyActorOnClicked(FKey ButtonPressed) override;

public:	

	UPROPERTY(EditAnywhere, Category = "GridCell")
	class UStaticMeshComponent* CellMesh;

	UPROPERTY(EditAnywhere, Category = "Materials")
	class UMaterialInterface* NormalMaterial;

	UPROPERTY(EditAnywhere, Category = "Materials")
	class UMaterialInterface* ObstacleMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GridCell")
	bool bIsOccupied;

	UPROPERTY(BlueprintAssignable, Category = "GridCell")
	FOnGridCellClicked OnGridCellClicked;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GridCell")
	UMaterialInterface* OriginalMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GridCell")
	bool bIsObstacle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GridCell")
	bool bWasObstacle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GridCell")
	UMaterialInterface* HighlightMaterial;

	UPROPERTY()
	class AGameManager* GameManagerRef;

	UFUNCTION()
	void HandleCursorBeginOver(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void HandleCursorEndOver(UPrimitiveComponent* TouchedComponent);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GridCell")
	int32 GridX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GridCell")
	int32 GridY;
};
