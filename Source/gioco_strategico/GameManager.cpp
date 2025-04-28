// Fill out your copyright notice in the Description page of Project Settings.
// ai correzione 2



#include "GameManager.h"
#include "GridManager.h"
#include "GridCell.h"
#include "Sniper.h"
#include "Brawler.h"
#include "UnitPawn.h"
#include "Containers/Queue.h"
#include "Containers/Set.h"
#include "Engine/World.h"
#include "Camera/CameraActor.h"
#include "EngineUtils.h" 
#include "DrawDebugHelpers.h" 
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include <Kismet/GameplayStatics.h>


AGameManager::AGameManager()
{
	
	
	GridCellSize = 100.f;
	GridHeight = 300.f;
	bIsPlayerFirst = false;
	DefaultPawnClass = nullptr;

	
	bPlayerSniperPlaced = false;
	bPlayerBrawlerPlaced = false;
	bAISniperPlaced = false;
	bAIBrawlerPlaced = false;
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> EnemyMatFinder(TEXT("/Game/texture/AttackMaterial.AttackMaterial"));
	if (EnemyMatFinder.Succeeded())
	{
		EnemyHighlightMaterial = EnemyMatFinder.Object;
	}

	static ConstructorHelpers::FClassFinder<UMyInterface> MyInterfaceBP(TEXT("WidgetBlueprint'/Game/blueprint/UI.UI_C'"));
	if (MyInterfaceBP.Succeeded())
	{
		MyInterfaceClass = MyInterfaceBP.Class;
	}
}

void AGameManager::BeginPlay()
{
	Super::BeginPlay();

	CurrentPhase = EGamePhase::Placement;

	
	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	
	if (PC && !MyInterfaceGame)
	{
		MyInterfaceGame = CreateWidget<UMyInterface>(PC, MyInterfaceClass);
		if (MyInterfaceGame)
		{
			MyInterfaceGame->AddToViewport();

			FText GameRules = FText::FromString(
				"Regole della Partita:\n"
				"1. Sono concesse 3 tipi di azioni mutualmente esclusive(movimento, attacco, movimento e attacco)\n"
				"2. Se si decide di non attaccare nonostante ci sia la possibilita', clickare sulla pedina per terminarne il turno\n"
				"3. Esiste il danno da controattaco per lo sniper se attacca un altro sniper o se attacca un brawler in una cella adiacente\n"
				"4. La partita termina quando uno tra Player o AI termina le proprie pedine "
			);
			MyInterfaceGame->ShowRules(GameRules);
			
			MyInterfaceGame->UpdateTurnText(FText::FromString("FASE DI POSIZIONAMENTO"));
		}
	}

	
	PerformCoinToss();

	
	SpawnGrid();
	if (GridManager)
	{
		for (AGridCell* Cell : GridManager->AllCells)
		{
			if (Cell)
			{
				Cell->OnGridCellClicked.AddDynamic(this, &AGameManager::OnGridCellClicked);
			}
		}
	}

	
	if (!bIsPlayerFirst)
	{
		
		bIsPlayerTurnToPlace = false;

		
		if (!bAISniperPlaced)
		{
			AUnitPawn* FirstAIUnit = SpawnAIUnit(); 
			if (FirstAIUnit)
			{
				
				TArray<AGridCell*> FreeCells;
				for (AGridCell* Cell : GridManager->AllCells)
				{
					if (Cell && !Cell->bIsOccupied && !Cell->bIsObstacle && !Cell->bWasObstacle)
					{
						FreeCells.Add(Cell);
					}
				}
				if (FreeCells.Num() > 0)
				{
					int32 RandIndex = FMath::RandRange(0, FreeCells.Num() - 1);
					PlaceUnitInCell(FreeCells[RandIndex], FirstAIUnit);
					UE_LOG(LogTemp, Log, TEXT("AI (vincitore coin toss) posiziona la PRIMA pedina nella cella (%d, %d)"),
						FreeCells[RandIndex]->GridX, FreeCells[RandIndex]->GridY);
				}
			}
		}

		
		bIsPlayerTurnToPlace = true;
	}
	else
	{
		
		bIsPlayerTurnToPlace = true;
	}

	
	SetupTopDownCamera();
	if (PC)
	{
		PC->bShowMouseCursor = true;
		PC->bEnableMouseOverEvents = true;
		PC->bEnableClickEvents = true; 
	}

}


void AGameManager::PerformCoinToss()
{
	bIsPlayerFirst = FMath::RandBool();
	if (bIsPlayerFirst)
	{
		UE_LOG(LogTemp, Warning, TEXT("Coin toss: Il giocatore posiziona per primo"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Coin toss: L'AI posiziona per prima"));
	}
}

void AGameManager::SpawnGrid()
{
	if (!GetWorld()) return;

	
	FActorSpawnParameters SpawnParams;
	GridManager = GetWorld()->SpawnActor<AGridManager>(AGridManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (GridManager)
	{
		
		GridManager->CellSize = GridCellSize;
		GridManager->GridHeight = GridHeight;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Errore nello spawn del GridManager"));
	}
}

void AGameManager::SetupTopDownCamera()
{
	if (!GetWorld()) return;


	float GridWidth = GridCellSize * GridManager->GridSizeX;
	float GridHeightInXY = GridCellSize * GridManager->GridSizeY;
	FVector CenterOfGrid = FVector(GridWidth / 2, GridHeightInXY / 2, 0.f);

	
	FVector CameraLocation = CenterOfGrid + FVector(0.f, 0.f, 2650.f);
	
	FRotator CameraRotation = FRotator(-90.f, 0.f, -90.f);

	FActorSpawnParameters SpawnParams;
	ACameraActor* TopDownCamera = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), CameraLocation, CameraRotation, SpawnParams);
	if (TopDownCamera)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			PC->SetViewTarget(TopDownCamera);
		}
	}
}

void AGameManager::OnGridCellClicked(AGridCell* ClickedCell)
{
	
	if (CurrentPhase == EGamePhase::Placement)
	{

		
		if (!ClickedCell || ClickedCell->bIsOccupied || ClickedCell->bIsObstacle || ClickedCell->bWasObstacle)
			return;

		
		if (!bPlayerSniperPlaced || !bPlayerBrawlerPlaced)
		{
			AUnitPawn* NewUnit = SpawnPlayerUnit();
			if (NewUnit)
			{
				PlaceUnitInCell(ClickedCell, NewUnit);
				UE_LOG(LogTemp, Log, TEXT("Il Player piazza un'unita' nella cella (%d, %d)."),
					ClickedCell->GridX, ClickedCell->GridY);
			}
		}

		
		bIsPlayerTurnToPlace = false;

		
		if (bPlayerSniperPlaced && bPlayerBrawlerPlaced && bAISniperPlaced && bAIBrawlerPlaced)
		{
			
			CurrentPhase = EGamePhase::Gameplay;
			UE_LOG(LogTemp, Log, TEXT("Fase di posizionamento terminata. Inizio della fase di gioco."));
			
			if (bIsPlayerFirst)
				StartPlayerTurn();
			else
				StartAITurn();

			return;
		}

		
		if (!bAISniperPlaced || !bAIBrawlerPlaced)
		{
			AUnitPawn* AIUnit = SpawnAIUnit(); 
			if (AIUnit)
			{
				
				TArray<AGridCell*> FreeCells;
				for (AGridCell* Cell : GridManager->AllCells)
				{
					if (Cell && !Cell->bIsOccupied && !Cell->bIsObstacle && !Cell->bWasObstacle)
					{
						FreeCells.Add(Cell);
					}
				}
				if (FreeCells.Num() > 0)
				{
					int32 RandIndex = FMath::RandRange(0, FreeCells.Num() - 1);
					PlaceUnitInCell(FreeCells[RandIndex], AIUnit);
					UE_LOG(LogTemp, Log, TEXT("L'AI piazza un'unita' automaticamente nella cella (%d, %d)."),
						FreeCells[RandIndex]->GridX, FreeCells[RandIndex]->GridY);
				}
			}
		}

		
		bIsPlayerTurnToPlace = true;
		
		if (bPlayerSniperPlaced && bPlayerBrawlerPlaced && bAISniperPlaced && bAIBrawlerPlaced)
		{
			
			CurrentPhase = EGamePhase::Gameplay;
			UE_LOG(LogTemp, Log, TEXT("Fase di posizionamento terminata. Inizio della fase di gioco."));
			
			if (bIsPlayerFirst)
			{
				StartPlayerTurn();
			}
			else
			{
				StartAITurn();
			}
		}
		return;
	}

	
	if (!bIsPlayerTurnToPlace && CurrentTurn != EGameTurn::Player)
	{
		return;
	}

	
	if (!ClickedCell || ClickedCell->bIsOccupied) return;

	
	if (ClickedCell->CellMesh->GetMaterial(0) == ClickedCell->HighlightMaterial)
	{
		
		ClickedCell->CellMesh->SetMaterial(0, ClickedCell->OriginalMaterial);
	}

	
	if (SelectedUnit)
	{
		
		if (SelectedUnit->bHasMovedThisTurn)
		{
			UE_LOG(LogTemp, Warning, TEXT("L'unità %s ha già mosso; non è consentito muoverla ulteriormente."), *SelectedUnit->GetName());
			return;
		}

		AttemptMoveUnit(SelectedUnit, ClickedCell);
		ClearHighlights();          
		bHoverEnabled = true;        
		return;
	}
	if (CurrentPhase == EGamePhase::EndGame)
	{
		return;
	}
}

AUnitPawn* AGameManager::SpawnPlayerUnit()
{

	AUnitPawn* Unit = nullptr;
	if (!bPlayerSniperPlaced)
	{
		Unit = GetWorld()->SpawnActor<ASniper>(ASniper::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		bPlayerSniperPlaced = true;
	}
	else if (!bPlayerBrawlerPlaced)
	{
		Unit = GetWorld()->SpawnActor<ABrawler>(ABrawler::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		bPlayerBrawlerPlaced = true;
	}
	if (Unit)
	{
		Unit->bIsPlayerTeam = true; 
		Unit->DisplayMaterial = Unit->GetDisplayMaterial(); 
		
		Unit->OnUnitClicked.AddDynamic(this, &AGameManager::OnPawnClicked);

		if (Unit->IsA(ASniper::StaticClass()))
		{
			Unit->Rename(TEXT("Player_sniper"));
		}
		else if (Unit->IsA(ABrawler::StaticClass()))
		{
			Unit->Rename(TEXT("Player_brawler"));
		}
	}
	return Unit;
}

AUnitPawn* AGameManager::SpawnAIUnit()
{
	AUnitPawn* Unit = nullptr;
	if (!bAISniperPlaced)
	{
		Unit = GetWorld()->SpawnActor<ASniper>(ASniper::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		bAISniperPlaced = true;
	}
	else if (!bAIBrawlerPlaced)
	{
		Unit = GetWorld()->SpawnActor<ABrawler>(ABrawler::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		bAIBrawlerPlaced = true;
	}
	if (Unit)
	{
		Unit->bIsPlayerTeam = false; 
		
		Unit->OnUnitClicked.AddDynamic(this, &AGameManager::OnPawnClicked);

		if (Unit->IsA(ASniper::StaticClass()))
		{
			Unit->Rename(TEXT("AI_sniper"));
		}
		else if (Unit->IsA(ABrawler::StaticClass()))
		{
			Unit->Rename(TEXT("AI_brawler"));
		}

	}
	return Unit;
}

void AGameManager::PlaceUnitInCell(AGridCell* Cell, AUnitPawn* Unit)
{
	if (!Cell || !Unit) return;

	
	if (Cell->CellMesh->GetMaterial(0) == Cell->HighlightMaterial)
	{
		Cell->CellMesh->SetMaterial(0, Cell->OriginalMaterial);
	}

	
	FVector NewLocation = Cell->GetActorLocation();
	NewLocation.Z += 10.f; 

	Unit->SetActorLocation(NewLocation);

	
	Unit->UnitMesh->SetMaterial(0, Unit->DisplayMaterial);

	
	Unit->Move(Cell->GridX, Cell->GridY);

	
	Cell->bIsOccupied = true;

	if (MyInterfaceGame)
	{
		
		if (Unit->bIsPlayerTeam)
		{
			if (Unit->IsA(ASniper::StaticClass()))
			{
				
				MyInterfaceGame->UpdateHealthBar_Player1(Unit->Health, Unit->MaxHealth);
			}
			else if (Unit->IsA(ABrawler::StaticClass()))
			{
				
				MyInterfaceGame->UpdateHealthBar_Player2(Unit->Health, Unit->MaxHealth);
			}
		}
		else
		{
			
			if (Unit->IsA(ASniper::StaticClass()))
			{
				MyInterfaceGame->UpdateHealthBar_AI1(Unit->Health, Unit->MaxHealth);
			}
			else if (Unit->IsA(ABrawler::StaticClass()))
			{
				MyInterfaceGame->UpdateHealthBar_AI2(Unit->Health, Unit->MaxHealth);
			}
		}
	}
}

void AGameManager::StartPlayerTurn()
{
	CurrentTurn = EGameTurn::Player;
	UE_LOG(LogTemp, Log, TEXT("Turno del Player iniziato"));

	
	if (MyInterfaceGame)
	{
		MyInterfaceGame->UpdateTurnText(FText::FromString("TURNO DEL PLAYER"));
	}

	
	for (TActorIterator<AUnitPawn> It(GetWorld()); It; ++It)
	{
		AUnitPawn* Pawn = *It;
		if (Pawn && Pawn->bIsPlayerTeam)
		{
			Pawn->bHasActedThisTurn = false;
			Pawn->bHasMovedThisTurn = false;
			Pawn->bHasAttackedThisTurn = false;
		}
	}
}

void AGameManager::StartAITurn()
{
	CurrentTurn = EGameTurn::AI;
	UE_LOG(LogTemp, Log, TEXT("Turno dell'AI iniziato"));

	if (MyInterfaceGame)
	{
		MyInterfaceGame->UpdateTurnText(FText::FromString("TURNO DEL AI"));
	}

	
	for (TActorIterator<AUnitPawn> It(GetWorld()); It; ++It)
	{
		AUnitPawn* Pawn = *It;
		if (Pawn && !Pawn->bIsPlayerTeam && Pawn->Health > 0)
		{
			Pawn->bHasActedThisTurn = false;
			Pawn->bHasMovedThisTurn = false;
			Pawn->bHasAttackedThisTurn = false;
		}
	}

	
	CurrentMovementData = FMovementData();

	
	ExecuteAIMove();
}

void AGameManager::EndTurn()
{

	if (bGameOver)
	{
		return;
	}

	
	UE_LOG(LogTemp, Log, TEXT("Turno terminato"));

	
	CurrentMovementData = FMovementData();
	SelectedUnit = nullptr;


	
	FTimerHandle TempHandle;
	if (CurrentTurn == EGameTurn::Player)
	{
		GetWorldTimerManager().SetTimer(TempHandle, this, &AGameManager::StartAITurn, 0.1f, false);
	}
	else
	{
		GetWorldTimerManager().SetTimer(TempHandle, this, &AGameManager::StartPlayerTurn, 0.1f, false);
	}
}

void AGameManager::ExecuteAIMove()
{
	
	if (CurrentMovementData.MovingUnit != nullptr)
	{
		return;
	}

	
	if (HaveAllAIUnitsDone())
	{
		
		FTimerHandle TempHandle;
		GetWorldTimerManager().SetTimer(TempHandle, this, &AGameManager::EndTurn, 0.1f, false);
		return;
	}

	bool bFoundUnit = false;
	
	for (TActorIterator<AUnitPawn> It(GetWorld()); It; ++It)
	{
		AUnitPawn* AIUnit = *It;
		if (AIUnit && !AIUnit->bIsPlayerTeam && !AIUnit->bHasActedThisTurn && AIUnit->Health > 0)
		{
			bFoundUnit = true;

			int32 UpdatedX = FMath::RoundToInt(AIUnit->GetActorLocation().X / GridCellSize);
			int32 UpdatedY = FMath::RoundToInt(AIUnit->GetActorLocation().Y / GridCellSize);
			AIUnit->Move(UpdatedX, UpdatedY);

			
			AUnitPawn* BestTarget = nullptr;
			TArray<AGridCell*> BestPath;
			int32 BestDistance = TNumericLimits<int32>::Max();
			AGridCell* StartCell = GridManager->GetCellAt(AIUnit->CurrentX, AIUnit->CurrentY);
			if (!StartCell)
			{
				AIUnit->bHasActedThisTurn = true;
				continue;
			}
			for (TActorIterator<AUnitPawn> PlayerIt(GetWorld()); PlayerIt; ++PlayerIt)
			{
				AUnitPawn* PlayerUnit = *PlayerIt;
				if (PlayerUnit && PlayerUnit->bIsPlayerTeam && PlayerUnit->Health > 0)
				{
					AGridCell* TargetCell = GridManager->GetCellAt(PlayerUnit->CurrentX, PlayerUnit->CurrentY);
					if (!TargetCell)
						continue;
					TArray<AGridCell*> TempPath;
					if (FindPath(StartCell, TargetCell, TempPath))
					{
						int32 Steps = TempPath.Num() - 1;
						if (Steps < BestDistance)
						{
							BestDistance = Steps;
							BestTarget = PlayerUnit;
							BestPath = TempPath;
						}
					}
				}
			}
			if (!BestTarget)
			{
				UE_LOG(LogTemp, Log, TEXT("%s non trova bersagli."), *AIUnit->GetName());
				AIUnit->bHasActedThisTurn = true;
				
				continue;
			}
			int32 ManhattanDistance = FMath::Abs(AIUnit->CurrentX - BestTarget->CurrentX) +
				FMath::Abs(AIUnit->CurrentY - BestTarget->CurrentY);
			if (ManhattanDistance <= AIUnit->AttackRange)
			{
				
				AttemptAttackUnit(AIUnit, BestTarget);
				AIUnit->bHasActedThisTurn = true;
				
				continue;
			}
			else
			{
				
				TArray<AGridCell*> CandidateCells;
				TArray<FIntPoint> Directions = { FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1) };
				for (const FIntPoint& Dir : Directions)
				{
					int32 CandidateX = BestTarget->CurrentX + Dir.X;
					int32 CandidateY = BestTarget->CurrentY + Dir.Y;
					AGridCell* CandidateCell = GridManager->GetCellAt(CandidateX, CandidateY);
					if (CandidateCell && !CandidateCell->bIsOccupied && !CandidateCell->bIsObstacle)
					{
						CandidateCells.Add(CandidateCell);
					}
				}
				AGridCell* BestDestination = nullptr;
				TArray<AGridCell*> BestCandidatePath;
				int32 BestCandidateDistance = TNumericLimits<int32>::Max();
				for (AGridCell* Candidate : CandidateCells)
				{
					TArray<AGridCell*> TempPath;
					if (FindPath(StartCell, Candidate, TempPath))
					{
						int32 Steps = TempPath.Num() - 1;
						if (Steps < BestCandidateDistance)
						{
							BestCandidateDistance = Steps;
							BestDestination = Candidate;
							BestCandidatePath = TempPath;
						}
					}
				}
				if (BestDestination)
				{
					int32 StepsToMove = FMath::Min(AIUnit->MovementRange, BestCandidatePath.Num() - 1);
					if (StepsToMove > 0)
					{
						AGridCell* DestinationCell = BestCandidatePath[StepsToMove];
						if (DestinationCell)
						{
							UE_LOG(LogTemp, Log, TEXT("%s si sposta dalla cella (%d, %d) a (%d, %d)"),
								*AIUnit->GetName(), StartCell->GridX, StartCell->GridY,
								DestinationCell->GridX, DestinationCell->GridY);
							AttemptMoveUnit(AIUnit, DestinationCell);
							
							return;
						}
					}
				}
				
				int32 StepsToMove = FMath::Min(AIUnit->MovementRange, BestPath.Num() - 1);
				if (StepsToMove > 0)
				{
					
					AGridCell* MoveCell = nullptr;
					for (int32 i = StepsToMove; i > 0; --i)
					{
						if (!BestPath[i]->bIsOccupied)
						{
							MoveCell = BestPath[i];
							break;
						}
					}
					if (MoveCell)
					{
						UE_LOG(LogTemp, Log, TEXT("%s si sposta dalla cella (%d, %d) a (%d, %d) [fallback]"),
							*AIUnit->GetName(), StartCell->GridX, StartCell->GridY,
							MoveCell->GridX, MoveCell->GridY);
						AttemptMoveUnit(AIUnit, MoveCell);
						return;
					}
				}
				
				ManhattanDistance = FMath::Abs(AIUnit->CurrentX - BestTarget->CurrentX) +
					FMath::Abs(AIUnit->CurrentY - BestTarget->CurrentY);
				if (ManhattanDistance <= AIUnit->AttackRange)
				{
					AttemptAttackUnit(AIUnit, BestTarget);
				}
				AIUnit->bHasActedThisTurn = true;
				
				continue;
			}
		}
	}
	
	if (HaveAllAIUnitsDone())
	{
		FTimerHandle TempHandle;
		GetWorldTimerManager().SetTimer(TempHandle, this, &AGameManager::EndTurn, 0.1f, false);
	}
}


bool AGameManager::FindPath(AGridCell* StartCell, AGridCell* EndCell, TArray<AGridCell*>& OutPath)
{
	if (!StartCell || !EndCell || !GridManager) return false;

	TQueue<AGridCell*> Queue;
	TMap<AGridCell*, AGridCell*> CameFrom;
	TSet<AGridCell*> Visited;

	Queue.Enqueue(StartCell);
	Visited.Add(StartCell);
	CameFrom.Add(StartCell, nullptr);

	bool bFound = false;
	while (!Queue.IsEmpty())
	{
		AGridCell* Current;
		Queue.Dequeue(Current);

		if (Current == EndCell)
		{
			bFound = true;
			break;
		}

		
		TArray<FIntPoint> Directions = { FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1) };
		for (const FIntPoint& Dir : Directions)
		{
			int32 NextX = Current->GridX + Dir.X;
			int32 NextY = Current->GridY + Dir.Y;
			AGridCell* Neighbor = GridManager->GetCellAt(NextX, NextY);
			if (Neighbor && !Visited.Contains(Neighbor))
			{
				
				bool bIsEndCell = (Neighbor == EndCell);
				bool bIgnoreOccupato = (Current == StartCell) || bIsEndCell;
				
				if (!Neighbor->bIsObstacle && (bIgnoreOccupato || !Neighbor->bIsOccupied))
				{
					Queue.Enqueue(Neighbor);
					Visited.Add(Neighbor);
					CameFrom.Add(Neighbor, Current);
				}
			}
		}
	}

	if (!bFound)
	{
		return false;
	}

	
	TArray<AGridCell*> Path;
	AGridCell* Current = EndCell;
	while (Current)
	{
		Path.Insert(Current, 0); 
		Current = CameFrom.Contains(Current) ? CameFrom[Current] : nullptr;
	}

	OutPath = Path;
	return true;
}

void AGameManager::AttemptMoveUnit(AUnitPawn* Unit, AGridCell* DestinationCell)
{
	if (!Unit || !DestinationCell || !GridManager) return;

	
	if (Unit->bHasMovedThisTurn)
	{
		UE_LOG(LogTemp, Warning, TEXT("L'unità %s ha già mosso in questo turno."), *Unit->GetName());
		return;
	}

	
	AGridCell* StartCell = GridManager->GetCellAt(Unit->CurrentX, Unit->CurrentY);
	if (!StartCell)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cella di partenza non trovata per l'unita'"));
		return;
	}

	TArray<AGridCell*> Path;
	if (FindPath(StartCell, DestinationCell, Path))
	{
		int32 Steps = Path.Num() - 1;
		if (Steps <= Unit->MovementRange)
		{
			
			StartCell->bIsOccupied = false;

			
			StartSmoothPathMovement(Unit, Path);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Destinazione troppo lontana: %d passi necessari, massimo %d"),
				Steps, Unit->MovementRange);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Nessun percorso valido trovato per l'unità %s"), *Unit->GetName());
	}
}

void AGameManager::AttemptAttackUnit(AUnitPawn* Attacker, AUnitPawn* Defender)
{
	if (!Attacker || !Defender) return;
	if (Attacker->bHasActedThisTurn) return; 

	
	int32 Distance = FMath::Abs(Attacker->CurrentX - Defender->CurrentX) +
		FMath::Abs(Attacker->CurrentY - Defender->CurrentY);
	if (Distance <= Attacker->AttackRange && IsValid(Defender))
	{
		
		int32 Damage = FMath::RandRange(Attacker->MinDamage, Attacker->MaxDamage);
		Defender->Health -= Damage;

		Defender->Health = FMath::Clamp(Defender->Health, 0.0f, Defender->MaxHealth);

		if (Defender->bIsPlayerTeam)
		{
			if (Defender->GetName().Contains("Player_sniper"))
			{
				if (MyInterfaceGame)
				{
					
					MyInterfaceGame->UpdateHealthBar_Player1(Defender->Health, Defender->MaxHealth);
				}
			}
			else if (Defender->GetName().Contains("Player_brawler"))
			{
				if (MyInterfaceGame)
				{
					MyInterfaceGame->UpdateHealthBar_Player2(Defender->Health, Defender->MaxHealth);
				}
			}
		}
		else
		{
			
			if (Defender->GetName().Contains("AI_sniper"))
			{
				if (MyInterfaceGame)
				{
					MyInterfaceGame->UpdateHealthBar_AI1(Defender->Health, Defender->MaxHealth);
				}
			}
			else if (Defender->GetName().Contains("AI_brawler"))
			{
				if (MyInterfaceGame)
				{
					MyInterfaceGame->UpdateHealthBar_AI2(Defender->Health, Defender->MaxHealth);
				}
			}
		}

		UE_LOG(LogTemp, Log, TEXT("%s attacca %s per %d danni"), *Attacker->GetName(), *Defender->GetName(), Damage);
		
		FString PlayerId = Attacker->bIsPlayerTeam ? TEXT("HP ") : TEXT("AI");
		FString UnitId = Attacker->IsA(ASniper::StaticClass()) ? TEXT("S") : TEXT("B");
		FString TargetCoord = ConvertGridCoordinate(Defender->CurrentX, Defender->CurrentY);
		FString AttackLogEntry = FString::Printf(TEXT("%s: %s %s %d"), *PlayerId, *UnitId, *TargetCoord, Damage);
		if (MyInterfaceGame)
		{
			MyInterfaceGame->AddHistoryEntry(FText::FromString(AttackLogEntry));
		}

		bool bDefenderAlive = Defender->Health > 0;
		if (Defender->Health <= 0)
		{
			AGridCell* DefenderCell = GridManager ? GridManager->GetCellAt(Defender->CurrentX, Defender->CurrentY) : nullptr;
			if (DefenderCell)
			{
				DefenderCell->bIsOccupied = false;
			}

			UE_LOG(LogTemp, Warning, TEXT("%s ha finito i punti vita e viene distrutta"), *Defender->GetName());
			Defender->Destroy();
			CheckForGameOver();
		}

		if (Attacker->IsA(ASniper::StaticClass()))
		{
			if (bDefenderAlive &&
				(Defender->IsA(ASniper::StaticClass()) ||
					(Defender->IsA(ABrawler::StaticClass()) && Distance == 1)))
			{
				int32 CounterDamage = FMath::RandRange(1, 3);
				Attacker->Health -= CounterDamage;
				UE_LOG(LogTemp, Log, TEXT("%s subisce un danno da contrattacco di %d"), *Attacker->GetName(), CounterDamage);
				
				Attacker->Health = FMath::Clamp(Attacker->Health, 0.0f, Attacker->MaxHealth);

				if (Attacker->bIsPlayerTeam)
				{
					if (Attacker->GetName().Contains(TEXT("Player_sniper")))
					{
						MyInterfaceGame->UpdateHealthBar_Player1(Attacker->Health, Attacker->MaxHealth);
					}
				}
				else
				{
					if (Attacker->GetName().Contains(TEXT("AI_sniper")))
					{
						MyInterfaceGame->UpdateHealthBar_AI1(Attacker->Health, Attacker->MaxHealth);
					}
				}
				
				if (Attacker->Health <= 0)
				{
					AGridCell* AttackerCell = GridManager ? GridManager->GetCellAt(Attacker->CurrentX, Attacker->CurrentY) : nullptr;
					if (AttackerCell)
					{
						AttackerCell->bIsOccupied = false;
					}

					UE_LOG(LogTemp, Log, TEXT("%s è stato eliminato dal contrattacco"), *Attacker->GetName());
					Attacker->Destroy();
					CheckForGameOver();
				}
			}
		}

		
		Attacker->bHasAttackedThisTurn = true;
		Attacker->bHasActedThisTurn = true;

		CheckForGameOver();

		
		if (CurrentTurn == EGameTurn::Player)
		{
			if (HaveAllPlayerUnitsDone())
			{
				EndTurn();
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Bersaglio fuori range."));
	}
}

void AGameManager::HighlightReachableCells(AUnitPawn* Unit)
{
	if (!Unit || !GridManager)
	{
		return;
	}
	
	if (Unit->bHasMovedThisTurn)
	{
		return;
	}

	AGridCell* StartCell = GridManager->GetCellAt(Unit->CurrentX, Unit->CurrentY);
	if (!StartCell)
	{
		return;
	}

	
	if (StartCell->CellMesh && StartCell->HighlightMaterial)
	{
		StartCell->CellMesh->SetMaterial(0, StartCell->HighlightMaterial);
	}

	
	for (AGridCell* Cell : GridManager->AllCells)
	{
		if (!Cell) continue;

		if (Cell->bIsObstacle)
		{
			continue;
		}

		
		if (Cell->bIsOccupied)
		{
			continue;
		}

		TArray<AGridCell*> Path;
		bool bPathFound = FindPath(StartCell, Cell, Path);
		if (!bPathFound)
		{
			continue;
		}

		int32 Steps = Path.Num() - 1;
		if (Steps <= Unit->MovementRange)
		{
			
			if (Cell->CellMesh && Cell->HighlightMaterial)
			{
				Cell->CellMesh->SetMaterial(0, Cell->HighlightMaterial);
			}
		}
	}
}

void AGameManager::OnPawnClicked(AUnitPawn* ClickedPawn)
{
	
	if (CurrentTurn != EGameTurn::Player)
	{
		UE_LOG(LogTemp, Log, TEXT("Non e' il turno del giocatore. Input ignorato."));
		return;
	}

	if (!ClickedPawn)
	{
		return;
	}

	
	bool bIsEnemy = !ClickedPawn->bIsPlayerTeam; 

	
	if (bIsEnemy && HighlightedEnemies.Contains(ClickedPawn))
	{
		
		if (SelectedUnit && !SelectedUnit->bHasAttackedThisTurn)
		{
			AttemptAttackUnit(SelectedUnit, ClickedPawn);
			SelectedUnit = nullptr;
			ClearHighlights();
			bHoverEnabled = true;
		}
		return;
	}

	
	if (!bIsEnemy)
	{
		
		if (ClickedPawn->bHasActedThisTurn)
		{
			UE_LOG(LogTemp, Log, TEXT("L'unità %s ha già agito in questo turno e non può essere selezionata."), *ClickedPawn->GetName());
			return;
		}

		
		if (SelectedUnit == ClickedPawn)
		{
			
			if (ClickedPawn->bHasMovedThisTurn && !ClickedPawn->bHasAttackedThisTurn)
			{
				ClickedPawn->bHasActedThisTurn = true;
				UE_LOG(LogTemp, Log, TEXT("L'unità %s ha concluso l'azione senza attaccare."), *ClickedPawn->GetName());
				SelectedUnit = nullptr;
				ClearHighlights();
				bHoverEnabled = true;

				
				if (HaveAllPlayerUnitsDone())
				{
					EndTurn();
				}
			}
			else
			{
				
				SelectedUnit = nullptr;
				ClearHighlights();
				bHoverEnabled = true;
				UE_LOG(LogTemp, Log, TEXT("Unità deselezionata: %s"), *ClickedPawn->GetName());
			}
			return;
		}
		else
		{
			
			SelectedUnit = ClickedPawn;
			ClearHighlights();

			
			if (!ClickedPawn->bHasMovedThisTurn)
			{
				HighlightReachableCells(ClickedPawn);
			}
			
			HighlightEnemiesInRange(ClickedPawn);

			bHoverEnabled = false;
			UE_LOG(LogTemp, Log, TEXT("Unità selezionata: %s"), *ClickedPawn->GetName());
		}
	}
	if (CurrentPhase == EGamePhase::EndGame)
	{
		return;
	}
}

void AGameManager::ClearHighlights()
{
	if (!GridManager) return;

	FlushPersistentDebugLines(GetWorld());

	for (AGridCell* Cell : GridManager->AllCells)
	{
		if (!Cell) continue;

		
		if (Cell->bIsObstacle || Cell->bWasObstacle)
		{
			
		}
		else
		{
			
			Cell->CellMesh->SetMaterial(0, Cell->OriginalMaterial);
		}
	}
}

bool AGameManager::HaveAllPlayerUnitsDone() const
{
	int32 CountPlayerUnits = 0;
	int32 DoneCount = 0;

	for (TActorIterator<AUnitPawn> It(GetWorld()); It; ++It)
	{
		AUnitPawn* Pawn = *It;
		if (Pawn && Pawn->bIsPlayerTeam && Pawn->Health > 0)
		{
			CountPlayerUnits++;
			if (Pawn->bHasActedThisTurn)
			{
				DoneCount++;
			}
		}
	}
	
	return (CountPlayerUnits > 0) && (CountPlayerUnits == DoneCount);
}

void AGameManager::HighlightEnemiesInRange(AUnitPawn* AttackingUnit)
{
	if (!AttackingUnit) return;


	
	bool bIsPlayer = AttackingUnit->bIsPlayerTeam;

	for (TActorIterator<AUnitPawn> It(GetWorld()); It; ++It)
	{
		AUnitPawn* PotentialEnemy = *It;
		if (!PotentialEnemy) continue;
		
		if ((PotentialEnemy->bIsPlayerTeam != bIsPlayer) && PotentialEnemy->Health > 0)
		{
			
			int32 Distance = FMath::Abs(AttackingUnit->CurrentX - PotentialEnemy->CurrentX) +
				FMath::Abs(AttackingUnit->CurrentY - PotentialEnemy->CurrentY);

			
			if (Distance <= AttackingUnit->AttackRange)
			{
				
				AGridCell* EnemyCell = GridManager->GetCellAt(PotentialEnemy->CurrentX, PotentialEnemy->CurrentY);
				if (EnemyCell)
				{
					
					FVector CenterLocation = EnemyCell->GetActorLocation() + FVector(0.f, 0.f, 1.f);
					float HalfSize = GridCellSize * 0.5f;
					FVector BoxExtent(HalfSize, HalfSize, 1.f);

					DrawDebugBox(
						GetWorld(),
						CenterLocation,
						BoxExtent,
						FQuat::Identity,
						FColor::Red,
						true,   
						0.f,    
						0,      
						7.f     
					);
					
					UE_LOG(LogTemp, Log, TEXT("Evidenzio cella del nemico a portata: %s"), *PotentialEnemy->GetName());

					
					HighlightedEnemies.Add(PotentialEnemy);
				}
			}
		}
	}
}

bool AGameManager::CanAttackFromCurrentPosition(AUnitPawn* Unit) const
{
	if (!Unit || !GridManager)
	{
		return false;
	}

	for (TActorIterator<AUnitPawn> It(GetWorld()); It; ++It)
	{
		AUnitPawn* Other = *It;
		
		if (Other && (Other->bIsPlayerTeam != Unit->bIsPlayerTeam) && (Other->Health > 0))
		{
			int32 Distance = FMath::Abs(Unit->CurrentX - Other->CurrentX) +
				FMath::Abs(Unit->CurrentY - Other->CurrentY);
			if (Distance <= Unit->AttackRange)
			{
				return true;
			}
		}
	}
	return false;
}

void AGameManager::StartSmoothPathMovement(AUnitPawn* Unit, const TArray<AGridCell*>& Path)
{
	if (!Unit || Path.Num() < 2)
	{
		
		return;
	}

	
	CurrentMovementData.MovingUnit = Unit;
	CurrentMovementData.Path = Path;
	CurrentMovementData.CurrentSegmentIndex = 0;
	CurrentMovementData.ElapsedTime = 0.f;
	CurrentMovementData.SegmentTravelTime = 0.f;

	
	SetupNextSegment(0);
}

void AGameManager::SetupNextSegment(int32 SegmentIndex)
{
	
	AGridCell* FromCell = CurrentMovementData.Path[SegmentIndex];
	AGridCell* ToCell = CurrentMovementData.Path[SegmentIndex + 1];

	
	FVector StartLoc = FromCell->GetActorLocation();
	StartLoc.Z += 10.f;
	FVector EndLoc = ToCell->GetActorLocation();
	EndLoc.Z += 10.f;

	
	CurrentMovementData.SegmentStart = StartLoc;
	CurrentMovementData.SegmentEnd = EndLoc;
	CurrentMovementData.ElapsedTime = 0.f; 

	
	float Dist = FVector::Dist(StartLoc, EndLoc);
	if (Dist < 1.f)
	{
		Dist = 1.f;
	}
	
	if (MovementSpeed > 0.f)
	{
		CurrentMovementData.SegmentTravelTime = Dist / MovementSpeed;
	}
	else
	{
		CurrentMovementData.SegmentTravelTime = 0.1f;
	}
}

void AGameManager::ContinuePathSegment(float DeltaTime)
{
	FMovementData& MoveData = CurrentMovementData;
	if (!MoveData.MovingUnit || MoveData.Path.Num() < 2)
	{
		return;
	}

	
	MoveData.ElapsedTime += DeltaTime;


	float Alpha = 0.f;
	if (MoveData.SegmentTravelTime > 0.f)
	{
		Alpha = MoveData.ElapsedTime / MoveData.SegmentTravelTime;
	}
	Alpha = FMath::Clamp(Alpha, 0.f, 1.f);

	
	FVector NewLoc = FMath::Lerp(MoveData.SegmentStart, MoveData.SegmentEnd, Alpha);
	MoveData.MovingUnit->SetActorLocation(NewLoc);

	
	if (Alpha >= 1.f)
	{
		
		int32 NextSegment = MoveData.CurrentSegmentIndex + 1;
		AGridCell* ToCell = MoveData.Path[NextSegment];
		MoveData.MovingUnit->Move(ToCell->GridX, ToCell->GridY);

		
		if (NextSegment < MoveData.Path.Num() - 1)
		{
			MoveData.CurrentSegmentIndex = NextSegment;
			SetupNextSegment(NextSegment);
		}
		else
		{
			
			ToCell->bIsOccupied = true;

			
			MoveData.MovingUnit->bHasMovedThisTurn = true;

			
			FString PlayerId = MoveData.MovingUnit->bIsPlayerTeam ? TEXT("HP") : TEXT("AI");
			
			FString UnitId = MoveData.MovingUnit->IsA(ASniper::StaticClass()) ? TEXT("S") : TEXT("B");
			
			FString StartCoord = ConvertGridCoordinate(MoveData.Path[0]->GridX, MoveData.Path[0]->GridY);
			
			FString EndCoord = ConvertGridCoordinate(ToCell->GridX, ToCell->GridY);
			FString MoveLogEntry = FString::Printf(TEXT("%s : %s %s -> %s"), *PlayerId, *UnitId, *StartCoord, *EndCoord);

			if (MyInterfaceGame)
			{
				MyInterfaceGame->AddHistoryEntry(FText::FromString(MoveLogEntry));
			}

			
			if (MoveData.MovingUnit->bIsPlayerTeam)
			{
				AUnitPawn* PlayerUnit = MoveData.MovingUnit;
				if (PlayerUnit->AttackRange > 0)
				{
					HighlightedEnemies.Empty();
					HighlightEnemiesInRange(PlayerUnit);

					if (HighlightedEnemies.Num() == 0)
					{
						PlayerUnit->bHasActedThisTurn = true;
						SelectedUnit = nullptr;
						ClearHighlights();
						bHoverEnabled = true;

						if (HaveAllPlayerUnitsDone())
						{
							EndTurn();
						}
					}
				}
				else
				{
					PlayerUnit->bHasActedThisTurn = true;
					SelectedUnit = nullptr;
					ClearHighlights();
					bHoverEnabled = true;
				}
			}
			else
			{
				AUnitPawn* MovedAIUnit = MoveData.MovingUnit;
				AUnitPawn* BestTarget = nullptr;
				int32 BestDistance = TNumericLimits<int32>::Max();
				for (TActorIterator<AUnitPawn> It(GetWorld()); It; ++It)
				{
					AUnitPawn* PlayerUnit = *It;
					if (PlayerUnit && PlayerUnit->bIsPlayerTeam && PlayerUnit->Health > 0)
					{
						int32 Distance = FMath::Abs(MovedAIUnit->CurrentX - PlayerUnit->CurrentX) +
							FMath::Abs(MovedAIUnit->CurrentY - PlayerUnit->CurrentY);
						if (Distance <= MovedAIUnit->AttackRange && Distance < BestDistance)
						{
							BestDistance = Distance;
							BestTarget = PlayerUnit;
						}
					}
				}
				if (BestTarget)
				{
					UE_LOG(LogTemp, Log, TEXT("%s attacca %s dopo il movimento"),
						*MovedAIUnit->GetName(), *BestTarget->GetName());
					AttemptAttackUnit(MovedAIUnit, BestTarget);
				}
				MovedAIUnit->bHasActedThisTurn = true;
				FTimerHandle TempHandle;
				GetWorldTimerManager().SetTimer(TempHandle, this, &AGameManager::ExecuteAIMove, 0.1f, false);
			}

			CurrentMovementData = FMovementData();
		}
	}
}

bool AGameManager::HaveAllAIUnitsDone() const
{
	int32 CountAIUnits = 0;
	int32 DoneCount = 0;
	for (TActorIterator<AUnitPawn> It(GetWorld()); It; ++It)
	{
		AUnitPawn* Pawn = *It;
		if (Pawn && !Pawn->bIsPlayerTeam && Pawn->Health > 0)
		{
			CountAIUnits++;
			if (Pawn->bHasActedThisTurn)
			{
				DoneCount++;
			}
		}
	}
	return (CountAIUnits > 0) && (CountAIUnits == DoneCount);
}

void AGameManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);


	
	if (CurrentMovementData.MovingUnit != nullptr)
	{
		ContinuePathSegment(DeltaSeconds);
	}
}

void AGameManager::CheckForGameOver()
{
	bool bPlayerAlive = false;
	bool bAIAlive = false;

	
	for (TActorIterator<AUnitPawn> It(GetWorld()); It; ++It)
	{
		AUnitPawn* Pawn = *It;
		if (Pawn && Pawn->Health > 0)
		{
			if (Pawn->bIsPlayerTeam)
			{
				bPlayerAlive = true;
			}
			else
			{
				bAIAlive = true;
			}
		}
	}

	
	if (!bPlayerAlive && !bAIAlive)
	{
		EndGame(EGameOutcome::Tie);
	}
	else if (!bPlayerAlive)
	{
		EndGame(EGameOutcome::AIWin);
	}
	else if (!bAIAlive)
	{
		EndGame(EGameOutcome::PlayerWin);
	}
}

void AGameManager::EndGame(EGameOutcome Outcome)
{
	if (bGameOver)
	{
		return; 
	}
	bGameOver = true;

	
	CurrentPhase = EGamePhase::EndGame;

	if (MyInterfaceGame)
	{
		switch (Outcome)
		{
		case EGameOutcome::PlayerWin:
			MyInterfaceGame->UpdateTurnText(FText::FromString("HAI VINTO"));
			UE_LOG(LogTemp, Warning, TEXT("La partita è finita: Vittoria del Player!"));
			break;
		case EGameOutcome::AIWin:
			MyInterfaceGame->UpdateTurnText(FText::FromString("HAI PERSO"));
			UE_LOG(LogTemp, Warning, TEXT("La partita è finita: Vittoria dell'AI!"));
			break;
		case EGameOutcome::Tie:
			MyInterfaceGame->UpdateTurnText(FText::FromString("PAREGGIO"));
			UE_LOG(LogTemp, Warning, TEXT("La partita è finita: Pareggio!"));
			break;
		default:
			break;
		}
	}

	
}

FString AGameManager::ConvertGridCoordinate(int32 X, int32 Y) const
{
	char Letter = 'A' + X;
	int32 Number = Y + 1;
	return FString::Printf(TEXT("%c%d"), Letter, Number);
}






