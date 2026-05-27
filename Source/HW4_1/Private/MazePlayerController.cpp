#include "MazePlayerController.h"
#include "MazeAgent.h"
#include "GrowingTreeMazeGenerator.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

void AMazePlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, this, &AMazePlayerController::SetInitialGoal, 0.5f, false);
}

void AMazePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    InputComponent->BindAction("ChangeGoal", IE_Pressed, this, &AMazePlayerController::OnChangeGoal);
}

void AMazePlayerController::SetInitialGoal()
{
    UWorld* World = GetWorld();
    if (!World) return;

    AGrowingTreeMazeGenerator* MazeGen = Cast<AGrowingTreeMazeGenerator>(
        UGameplayStatics::GetActorOfClass(World, AGrowingTreeMazeGenerator::StaticClass()));
    AMazeAgent* Agent = Cast<AMazeAgent>(
        UGameplayStatics::GetActorOfClass(World, AMazeAgent::StaticClass()));

    FIntPoint GoalCell = MazeGen->GetGoalCell();
    Agent->UpdateGoal(GoalCell, MazeGen);
}

void AMazePlayerController::OnChangeGoal()
{
    UWorld* World = GetWorld();
    if (!World) return;

    AGrowingTreeMazeGenerator* MazeGen = Cast<AGrowingTreeMazeGenerator>(
        UGameplayStatics::GetActorOfClass(World, AGrowingTreeMazeGenerator::StaticClass()));
    AMazeAgent* Agent = Cast<AMazeAgent>(
        UGameplayStatics::GetActorOfClass(World, AMazeAgent::StaticClass()));
    

    const int32 X = FMath::RandRange(0, MazeGen->GridWidth - 1);
    const int32 Y = FMath::RandRange(0, MazeGen->GridHeight - 1);
    const FIntPoint NewGoal(X, Y);

    Agent->UpdateGoal(NewGoal, MazeGen);
}