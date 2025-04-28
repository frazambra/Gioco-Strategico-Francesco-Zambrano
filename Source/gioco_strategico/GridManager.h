// Fill out your copyright notice in the Description page of Project Settings.
//prova
//movim
#pragma once
#include "GridCell.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridManager.generated.h"

UCLASS()
class GIOCO_STRATEGICO_API AGridManager : public AActor
{
	GENERATED_BODY()

public:
	
	AGridManager();

protected:
	
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* ObstacleMaterial;

public:

	UPROPERTY(EditAnywhere, Category = "Grid")
	int32 GridSizeX = 25;

	UPROPERTY(EditAnywhere, Category = "Grid")
	int32 GridSizeY = 25;

	UPROPERTY(EditAnywhere, Category = "Grid")
	float CellSize = 100.0f; 

	UPROPERTY(EditAnywhere, Category = "Grid")
	float GridHeight = 2.f;

	UPROPERTY(EditAnywhere, Category = "Obstacles")
	UMaterialInterface* MountainMaterial;

	UPROPERTY(EditAnywhere, Category = "Obstacles")
	UMaterialInterface* Tree1Material;

	UPROPERTY(EditAnywhere, Category = "Obstacles")
	UMaterialInterface* Tree2Material;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	TArray<AGridCell*> AllCells;

	UFUNCTION(BlueprintCallable, Category = "Grid")
	AGridCell* GetCellAt(int32 X, int32 Y) const;


private:

	void GenerateGrid();
	
	void FloodFill(int32 X, int32 Y, TArray<TArray<bool>>& Visited, TArray<TArray<AGridCell*>>& Grid);

	void FixIsolatedCells(TArray<TArray<bool>>& Visited, TArray<TArray<AGridCell*>>& Grid, int32 StartX, int32 StartY);
};
