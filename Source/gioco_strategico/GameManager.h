// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyInterface.h"
#include "GameManager.generated.h"




UENUM(BlueprintType)
enum class EGameTurn : uint8
{
    Player UMETA(DisplayName = "Player"),
    AI     UMETA(DisplayName = "AI")
};

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	Placement UMETA(DisplayName = "Placement"),
	Gameplay  UMETA(DisplayName = "Gameplay"),
	EndGame   UMETA(DisplayName = "EndGame")
};

UENUM(BlueprintType)
enum class EGameOutcome : uint8
{
	None       UMETA(DisplayName = "None"),
	PlayerWin  UMETA(DisplayName = "PlayerWin"),
	AIWin      UMETA(DisplayName = "AIWin"),
	Tie        UMETA(DisplayName = "Tie")
};


USTRUCT()
struct FMovementData
{
	GENERATED_BODY()

	FMovementData()
		: MovingUnit(nullptr)
		, CurrentSegmentIndex(0)
		, ElapsedTime(0.f)
		, SegmentTravelTime(0.f)
		, SegmentStart(FVector::ZeroVector)
		, SegmentEnd(FVector::ZeroVector)
	{
	}

	UPROPERTY()
	class AUnitPawn* MovingUnit;

	UPROPERTY()
	TArray<AGridCell*> Path;

	int32 CurrentSegmentIndex;

	float ElapsedTime;

	float SegmentTravelTime;

	FVector SegmentStart;
	FVector SegmentEnd;
};


UCLASS()
class GIOCO_STRATEGICO_API AGameManager : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AGameManager();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
	bool bIsPlayerFirst;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Grid")
	float GridCellSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Grid")
	float GridHeight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	class AGridManager* GridManager;

	UPROPERTY(BlueprintReadWrite, Category = "Game")
	bool bHoverEnabled = true;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MovementSpeed = 300.f;

	bool bPlayerSniperPlaced;
	bool bPlayerBrawlerPlaced;
	bool bAISniperPlaced;
	bool bAIBrawlerPlaced;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
	bool bIsPlayerTurnToPlace;

	UFUNCTION(BlueprintCallable, Category = "Game")
	void HighlightReachableCells(AUnitPawn* Unit);

	UFUNCTION(BlueprintCallable, Category = "Game")
	void EndGame(EGameOutcome Outcome);

	UPROPERTY()
	UMyInterface* MyInterfaceGame;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UMyInterface> MyInterfaceClass;

protected:

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Game")
	void PerformCoinToss();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void SpawnGrid();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void SetupTopDownCamera();

	UFUNCTION()
	void OnGridCellClicked(class AGridCell* ClickedCell);

	void PlaceUnitInCell(class AGridCell* Cell, class AUnitPawn* Unit);

	class AUnitPawn* SpawnPlayerUnit();
	class AUnitPawn* SpawnAIUnit();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
	EGameTurn CurrentTurn;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
	EGamePhase CurrentPhase = EGamePhase::Placement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Materials")
	UMaterialInterface* EnemyHighlightMaterial;

	UPROPERTY()
	TArray<AUnitPawn*> HighlightedEnemies;

	UFUNCTION(BlueprintCallable, Category = "Game")
	void StartPlayerTurn();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void StartAITurn();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void EndTurn();

	UFUNCTION()
	void OnPawnClicked(class AUnitPawn* ClickedPawn);

	void ExecuteAIMove();

	FTimerHandle MovementTimerHandle;

	FMovementData CurrentMovementData;
	
	void StartSmoothPathMovement(class AUnitPawn* Unit, const TArray<class AGridCell*>& Path);

	void SetupNextSegment(int32 SegmentIndex);

	void ContinuePathSegment(float DeltaTime);

	bool HaveAllAIUnitsDone() const;

	void CheckForGameOver();


private:

	
	bool FindPath(AGridCell* StartCell, AGridCell* EndCell, TArray<AGridCell*>& OutPath);

	void AttemptMoveUnit(AUnitPawn* Unit, AGridCell* DestinationCell);

	void AttemptAttackUnit(AUnitPawn* Attacker, AUnitPawn* Defender);

	void HighlightEnemiesInRange(AUnitPawn* AttackingUnit);

	UPROPERTY()
	AUnitPawn* SelectedUnit = nullptr;

	void ClearHighlights();

	bool HaveAllPlayerUnitsDone() const;

	bool CanAttackFromCurrentPosition(AUnitPawn* Unit) const;

	bool bGameOver = false;

	FString ConvertGridCoordinate(int32 X, int32 Y) const;
};
