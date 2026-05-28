#include "MazeAgent.h"
#include "GrowingTreeMazeGenerator.h"
#include "MazePathfinder.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

AMazeAgent::AMazeAgent()
{
    PrimaryActorTick.bCanEverTick = true;
    VisualComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualComponent"));
    RootComponent = VisualComponent;
}

void AMazeAgent::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!bMoving || PathPoints.Num() == 0) return;

    FVector CurrentPos = GetActorLocation();
    FVector TargetPos = PathPoints[CurrentTargetIndex];
    FVector Direction = TargetPos - CurrentPos;
    float Distance = Direction.Size();

    if (Distance < 1.0f)
    {
        MoveToNextTarget();
        if (!bMoving) return;
        TargetPos = PathPoints[CurrentTargetIndex];
        Direction = TargetPos - CurrentPos;
        Distance = Direction.Size();
    }

    FVector MoveDelta = Direction.GetSafeNormal() * Speed * DeltaTime;
    if (MoveDelta.Size() > Distance)
        MoveDelta = Direction;

    SetActorLocation(CurrentPos + MoveDelta);
}

void AMazeAgent::MoveToNextTarget()
{
    CurrentTargetIndex++;
    if (CurrentTargetIndex >= PathPoints.Num())
    {
        bMoving = false;
        OnPathCompleted.Broadcast();
    }
}

void AMazeAgent::SetPath(const TArray<FIntPoint>& NewPath, float CellSize)
{
    PathPoints.Empty();
    for (const FIntPoint& Pt : NewPath)
    {
        FVector WorldPos((Pt.X + 0.5f) * CellSize, (Pt.Y + 0.5f) * CellSize, 50.0f);
        PathPoints.Add(WorldPos);
    }
    if (PathPoints.Num() > 0)
    {
        SetActorLocation(PathPoints[0]);
        CurrentTargetIndex = 1;
        bMoving = (PathPoints.Num() > 1);
    }
    else
    {
        bMoving = false;
    }
}

void AMazeAgent::StopMovement()
{
    bMoving = false;
}

void AMazeAgent::UpdateGoal(const FIntPoint& NewGoal, AGrowingTreeMazeGenerator* MazeGen)
{
    if (!MazeGen) return;
    UMazePathfinder* PF = MazeGen->GetPathfinder();
    if (!PF) return;
    
    CurrentGoal = NewGoal;
    bHasGoal = true;

    const float CellSize = MazeGen->GetCellSize();
    FIntPoint StartCell = GetCurrentCell(CellSize);
    StartCell.X = FMath::Clamp(StartCell.X, 0, MazeGen->GridWidth - 1);
    StartCell.Y = FMath::Clamp(StartCell.Y, 0, MazeGen->GridHeight - 1);

    TArray<FIntPoint> NewPath;
    TArray<FVisualStep> Steps;
    bool bFound = PF->FindPath(StartCell, NewGoal, NewPath, &Steps);
    
    StopMovement();
    UWorld* World = GetWorld();
    if (World)
    {
        FlushPersistentDebugLines(World);
        World->GetTimerManager().ClearTimer(VisualizationTimer);
        
        FVector GoalCenter((NewGoal.X + 0.5f) * CellSize, (NewGoal.Y + 0.5f) * CellSize, 70.0f);
        DrawDebugSphere(World, GoalCenter, CellSize * 0.25f, 12, FColor::Red, true, -1.0f);
    }

    if (bFound)
    {
        PendingPath = NewPath;
        PendingCellSize = CellSize;
        VisualizeSearchSteps(Steps);
    }
}

void AMazeAgent::OnMazeRegenerated(AGrowingTreeMazeGenerator* MazeGen)
{
    if (!MazeGen) return;
    if (bHasGoal)
    {
        UpdateGoal(CurrentGoal, MazeGen);
    }
    else
    {
        UpdateGoal(MazeGen->GetGoalCell(), MazeGen);
    }
}

FIntPoint AMazeAgent::GetCurrentCell(float CellSize) const
{
    FVector Pos = GetActorLocation();
    int32 X = FMath::FloorToInt(Pos.X / CellSize);
    int32 Y = FMath::FloorToInt(Pos.Y / CellSize);
    return FIntPoint(X, Y);
}

void AMazeAgent::VisualizeSearchSteps(const TArray<FVisualStep>& Steps)
{
    if (Steps.Num() == 0)
    {
        FinishVisualization(PendingPath, PendingCellSize);
        return;
    }

    CurrentStepIndex = 0;
    PendingSteps = Steps;

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(VisualizationTimer, this, &AMazeAgent::ProcessNextStep, VisualizationStepDelay, true);
    }
}

void AMazeAgent::ProcessNextStep()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (CurrentStepIndex >= PendingSteps.Num())
    {
        World->GetTimerManager().ClearTimer(VisualizationTimer);
        FinishVisualization(PendingPath, PendingCellSize);
        return;
    }

    const FVisualStep& Step = PendingSteps[CurrentStepIndex];
    float CellSize = PendingCellSize;
    FVector Center((Step.Cell.X + 0.5f) * CellSize, (Step.Cell.Y + 0.5f) * CellSize, 50.0f);

    FColor Color = (Step.StepType == EVisStepType::OpenSetAdd) ? FColor::Cyan : FColor::Silver;
    DrawDebugSphere(World, Center, CellSize * 0.15f, 8, Color, false, 1.0f);

    CurrentStepIndex++;
}

void AMazeAgent::FinishVisualization(const TArray<FIntPoint>& Path, float CellSize)
{
    if (UWorld* World = GetWorld())
    {
        for (const FIntPoint& Pt : Path)
        {
            FVector Center((Pt.X + 0.5f) * CellSize, (Pt.Y + 0.5f) * CellSize, 70.0f);
            DrawDebugSphere(World, Center, CellSize * 0.15f, 8, FColor::Green, true, -1.0f);
        }
        for (int32 i = 0; i < Path.Num() - 1; ++i)
        {
            FVector A((Path[i].X + 0.5f) * CellSize, (Path[i].Y + 0.5f) * CellSize, 70.0f);
            FVector B((Path[i + 1].X + 0.5f) * CellSize, (Path[i + 1].Y + 0.5f) * CellSize, 70.0f);
            DrawDebugLine(World, A, B, FColor::Green, true, -1.0f, 0, 5.0f);
        }
    }
    SetPath(Path, CellSize);
}