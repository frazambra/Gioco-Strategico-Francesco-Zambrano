// Fill out your copyright notice in the Description page of Project Settings.

#include "GridCell.h"
#include "Kismet/GameplayStatics.h"
#include "GameManager.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"


AGridCell::AGridCell()
{
 	
	PrimaryActorTick.bCanEverTick = false;

	CellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CellMesh"));
	RootComponent = CellMesh;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMeshFinder(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Plane.Plane'"));
    if (PlaneMeshFinder.Succeeded())
    {
        CellMesh->SetStaticMesh(PlaneMeshFinder.Object);
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> NormalMatFinder(TEXT("/Script/Engine.Material'/Engine/EngineMaterials/DefaultDecalMaterial.DefaultDecalMaterial'"));
    if (NormalMatFinder.Succeeded())
    {
        NormalMaterial = NormalMatFinder.Object;
        CellMesh->SetMaterial(0, NormalMaterial);
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> ObstacleMatFinder(TEXT("/Script/Engine.Material'/Engine/EngineMaterials/EmissiveTexturedMaterial.EmissiveTexturedMaterial'"));
    if (ObstacleMatFinder.Succeeded())
    {
        ObstacleMaterial = ObstacleMatFinder.Object;
    }

    CellMesh->SetRelativeScale3D(FVector(0.95f, 0.95f, 1.0f));

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> HighlightMatFinder(TEXT("/Game/texture/CursorMaterial.CursorMaterial"));
    if (HighlightMatFinder.Succeeded())
    {
        HighlightMaterial = HighlightMatFinder.Object;
    }
    CellMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CellMesh->SetCollisionObjectType(ECC_WorldDynamic);
    CellMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    CellMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    CellMesh->SetGenerateOverlapEvents(true);

    bIsOccupied = false;
    bIsObstacle = false;
    bWasObstacle = false;
    GridX = 0;
    GridY = 0;
}


void AGridCell::BeginPlay()
{
	Super::BeginPlay();

    GameManagerRef = Cast<AGameManager>(UGameplayStatics::GetGameMode(GetWorld()));

    if (CellMesh)
    {

        CellMesh->OnBeginCursorOver.AddDynamic(this, &AGridCell::HandleCursorBeginOver);
        CellMesh->OnEndCursorOver.AddDynamic(this, &AGridCell::HandleCursorEndOver);
        OriginalMaterial = CellMesh->GetMaterial(0);
    }
	
}

void AGridCell::NotifyActorOnClicked(FKey ButtonPressed)
{
    Super::NotifyActorOnClicked(ButtonPressed);
    UE_LOG(LogTemp, Warning, TEXT("Cell clicked: (%d, %d)"), GridX, GridY);
    OnGridCellClicked.Broadcast(this);
}

void AGridCell::HandleCursorBeginOver(UPrimitiveComponent* TouchedComponent)
{
    if (!GameManagerRef || !GameManagerRef->bHoverEnabled)
    {
        return;
    }

    if (bIsOccupied)
    {
        return;
    }

    if (bWasObstacle || bIsObstacle)
    {
        return;
    }

    if (CellMesh && HighlightMaterial)
    {
        CellMesh->SetMaterial(0, HighlightMaterial);
    }
}

void AGridCell::HandleCursorEndOver(UPrimitiveComponent* TouchedComponent)
{
    if (!GameManagerRef || !GameManagerRef->bHoverEnabled)
    {
        return;
    }

    if (bIsOccupied)
    {
        return;
    }

    if (bWasObstacle || bIsObstacle)
    {

        return;
    }

    if (CellMesh && NormalMaterial)
    {
        CellMesh->SetMaterial(0, NormalMaterial);
    }
}

