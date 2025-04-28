// Fill out your copyright notice in the Description page of Project Settings.


#include "GridManager.h"
#include "Engine/World.h"
#include "GridCell.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"


AGridManager::AGridManager()
{

	PrimaryActorTick.bCanEverTick =false;

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatMountain(
        TEXT("/Game/texture/montagna.montagna")
    );

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatTree1(
        TEXT("/Game/texture/albero.albero")
    );

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatTree2(
        TEXT("/Game/texture/alberello.alberello")
    );

    if (MatMountain.Succeeded())
    {
        MountainMaterial = MatMountain.Object;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("MatMountain NOT found! Check path."));
    }

    if (MatTree1.Succeeded())
    {
        Tree1Material = MatTree1.Object;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("MatTree1 NOT found! Check path."));
    }

    if (MatTree2.Succeeded())
    {
        Tree2Material = MatTree2.Object;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("MatTree2 NOT found! Check path."));
    }
}


void AGridManager::BeginPlay()
{
	Super::BeginPlay();
	GenerateGrid();
}

void AGridManager::FloodFill(int32 X, int32 Y, TArray<TArray<bool>>& Visited, TArray<TArray<AGridCell*>>& Grid)
{
    
    if (X < 0 || X >= GridSizeX || Y < 0 || Y >= GridSizeY || Visited[X][Y])
        return;

    
    if (Grid[X][Y]->CellMesh->GetMaterial(0) != Grid[X][Y]->NormalMaterial)
        return;

    
    Visited[X][Y] = true;

    
    FloodFill(X + 1, Y, Visited, Grid);
    FloodFill(X - 1, Y, Visited, Grid);
    FloodFill(X, Y + 1, Visited, Grid);
    FloodFill(X, Y - 1, Visited, Grid);
}

void AGridManager::GenerateGrid()
{
    TArray<TArray<AGridCell*>> Grid;
    Grid.SetNum(GridSizeX);

    
    for (int32 X = 0; X < GridSizeX; X++)
    {
        Grid[X].SetNum(GridSizeY);
        for (int32 Y = 0; Y < GridSizeY; Y++)
        {
            FVector SpawnLocation = FVector(X * CellSize, Y * CellSize, GridHeight);
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;

            AGridCell* NewCell = GetWorld()->SpawnActor<AGridCell>(
                AGridCell::StaticClass(),
                SpawnLocation,
                FRotator::ZeroRotator,
                SpawnParams
            );

           
            if (NewCell)
            {
                NewCell->GridX = X;
                NewCell->GridY = Y;
            }

            bool bIsObstacle = FMath::RandRange(0, 100) < 10;
            if (bIsObstacle && NewCell)
            {
                
                NewCell->bIsObstacle = true;
                NewCell->bWasObstacle = true; 

                int32 Index = FMath::RandRange(0, 2);
                UMaterialInterface* RandomMat = nullptr;

                switch (Index)
                {
                case 0:
                    RandomMat = MountainMaterial;
                    break;
                case 1:
                    RandomMat = Tree1Material;
                    break;
                case 2:
                    RandomMat = Tree2Material;
                    break;
                }

                if (RandomMat)
                {
                    NewCell->CellMesh->SetMaterial(0, RandomMat);
                }
                
               
            }

            Grid[X][Y] = NewCell;

           
            AllCells.Add(NewCell);
        }
    }

    
    TArray<TArray<bool>> Visited;
    Visited.SetNum(GridSizeX);
    for (int32 X = 0; X < GridSizeX; X++)
    {
        Visited[X].SetNum(GridSizeY);
        for (int32 Y = 0; Y < GridSizeY; Y++)
        {
            Visited[X][Y] = false;
        }
    }

   
    int32 StartX = -1, StartY = -1;
    bool bFoundStart = false;
    for (int32 X = 0; X < GridSizeX && !bFoundStart; X++)
    {
        for (int32 Y = 0; Y < GridSizeY && !bFoundStart; Y++)
        {
            if (Grid[X][Y]->CellMesh->GetMaterial(0) == Grid[X][Y]->NormalMaterial)
            {
                StartX = X;
                StartY = Y;
                bFoundStart = true;
            }
        }
    }

    if (!bFoundStart)
    {
        return;
    }

    int32 Attempts = 0;
    const int32 MaxAttempts = 1000;
    bool bIsConnected = false;

    
    while (!bIsConnected && Attempts < MaxAttempts)
    {
        
        for (int32 X = 0; X < GridSizeX; X++)
        {
            for (int32 Y = 0; Y < GridSizeY; Y++)
            {
                Visited[X][Y] = false;
            }
        }

        
        FloodFill(StartX, StartY, Visited, Grid);

   
    bool bHasIsolatedCells = false;
    for (int32 X = 0; X < GridSizeX; X++)
    {
        for (int32 Y = 0; Y < GridSizeY; Y++)
        {
            if (Grid[X][Y]->CellMesh->GetMaterial(0) == Grid[X][Y]->NormalMaterial && !Visited[X][Y])
            {
                bHasIsolatedCells = true;
                break;
            }
        }
        if (bHasIsolatedCells)
            break;
    }

    if (bHasIsolatedCells)
    {
        FixIsolatedCells(Visited, Grid, StartX, StartY);
    }
    else
    {
        bIsConnected = true;
    }
    Attempts++;
    }

    if (!bIsConnected)
    {
        UE_LOG(LogTemp, Warning, TEXT("Impossibile correggere la connettività delle celle normali dopo %d tentativi."), Attempts);
    }
}

void AGridManager::FixIsolatedCells(TArray<TArray<bool>>& Visited, TArray<TArray<AGridCell*>>& Grid, int32 StartX, int32 StartY)
{
   
    for (int32 X = 0; X < GridSizeX; X++)
    {
        for (int32 Y = 0; Y < GridSizeY; Y++)
        {
            if (Grid[X][Y]->CellMesh->GetMaterial(0) == Grid[X][Y]->NormalMaterial && !Visited[X][Y])
            {
                
                TArray<FIntPoint> Directions = { FIntPoint(1,0), FIntPoint(-1,0), FIntPoint(0,1), FIntPoint(0,-1) };
                for (FIntPoint Dir : Directions)
                {
                    int32 NeighborX = X + Dir.X;
                    int32 NeighborY = Y + Dir.Y;
                    if (NeighborX >= 0 && NeighborX < GridSizeX && NeighborY >= 0 && NeighborY < GridSizeY)
                    {
                        
                        if (Grid[NeighborX][NeighborY]->CellMesh->GetMaterial(0) != Grid[NeighborX][NeighborY]->NormalMaterial)
                        {
                            
                            Grid[NeighborX][NeighborY]->CellMesh->SetMaterial(0, Grid[NeighborX][NeighborY]->NormalMaterial);

                            
                            Grid[NeighborX][NeighborY]->bIsObstacle = false;
                            

                            
                            int32 RandX, RandY;
                            do
                            {
                                RandX = FMath::RandRange(0, GridSizeX - 1);
                                RandY = FMath::RandRange(0, GridSizeY - 1);
                            } while (Grid[RandX][RandY]->CellMesh->GetMaterial(0) != Grid[RandX][RandY]->NormalMaterial);

                           
                            int32 Index = FMath::RandRange(0, 2);
                            UMaterialInterface* RandomObstacleMaterial = nullptr;
                            switch (Index)
                            {
                            case 0:
                                RandomObstacleMaterial = MountainMaterial;
                                break;
                            case 1:
                                RandomObstacleMaterial = Tree1Material;
                                break;
                            case 2:
                                RandomObstacleMaterial = Tree2Material;
                                break;
                            }
                            if (RandomObstacleMaterial)
                            {
                                Grid[RandX][RandY]->CellMesh->SetMaterial(0, RandomObstacleMaterial);
                                
                                Grid[RandX][RandY]->bIsObstacle = true;
                                Grid[RandX][RandY]->bWasObstacle = true;
                                
                            }

                           
                            return;
                        }
                    }
                }
            }
        }
    }
}

AGridCell* AGridManager::GetCellAt(int32 X, int32 Y) const
{
    for (AGridCell* Cell : AllCells)
    {
        if (Cell && Cell->GridX == X && Cell->GridY == Y)
        {
            return Cell;
        }
    }
    return nullptr;
}
