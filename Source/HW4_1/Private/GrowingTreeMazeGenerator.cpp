#include "GrowingTreeMazeGenerator.h"
#include "MazePathfinder.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInterface.h"
#include "DrawDebugHelpers.h"
#include "Components/InstancedStaticMeshComponent.h"

void AGrowingTreeMazeGenerator::BeginPlay()
{
    Super::BeginPlay();
    GenerateMaze();
}

AGrowingTreeMazeGenerator::AGrowingTreeMazeGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = RootSceneComponent;
    FloorMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("FloorMesh"));
    FloorMeshComponent->SetupAttachment(RootComponent);
    WallInstancedMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallInstances"));
    WallInstancedMeshComponent->SetupAttachment(RootComponent);
}

void AGrowingTreeMazeGenerator::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    GenerateMaze();
}

void AGrowingTreeMazeGenerator::GenerateMaze()
{
    ClearMaze();

    if (RandomSeed == 0)
        RandomStream.GenerateNewSeed();
    else
        RandomStream.Initialize(RandomSeed);

    RunGrowingTreeAlgorithm();
    ChooseEntranceAndExit();
    BuildMazeVisuals();
    
    if (EnsurePathfinder())
    {
        Pathfinder->Initialize(GridWidth, GridHeight, CellSize, WallHeight, Grid);
        
        FIntPoint Start = GetStartCell();
        FIntPoint End   = GetGoalCell();
        TArray<FIntPoint> Path;

        if (bVisualizePath)
        {
            FlushPersistentDebugLines(GetWorld());
            if (Pathfinder->FindPath(Start, End, Path))
            {
                Pathfinder->VisualizePath(Path, PathDebugDuration);
            }
        }
    }
    
    OnMazeRegenerated.Broadcast(this);
}

#if WITH_EDITOR
void AGrowingTreeMazeGenerator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    GenerateMaze();
}
#endif

void AGrowingTreeMazeGenerator::ClearMaze()
{
    Grid.Empty();
    Pathfinder = nullptr;
    if (FloorMeshComponent) FloorMeshComponent->ClearAllMeshSections();
    if (WallInstancedMeshComponent) WallInstancedMeshComponent->ClearInstances();
    FlushPersistentDebugLines(GetWorld());
}

void AGrowingTreeMazeGenerator::RunGrowingTreeAlgorithm()
{
    Grid.SetNum(GridHeight);
    for (int32 Y = 0; Y < GridHeight; ++Y)
    {
        Grid[Y].SetNum(GridWidth);
        for (int32 X = 0; X < GridWidth; ++X)
        {
            Grid[Y][X] = FCell();
        }
    }

    int32 StartX = 0;
    int32 StartY = 0;
    Grid[StartY][StartX].bVisited = true;
    TArray<FIntPoint> ActiveList;
    ActiveList.Add(FIntPoint(StartX, StartY));

    while (ActiveList.Num() > 0)
    {
        int32 ChosenIndex;
        switch (GrowingTreeStrategy)
        {
        case 0: ChosenIndex = GetRandomInt(0, ActiveList.Num() - 1); break;
        case 1: ChosenIndex = ActiveList.Num() - 1; break;
        default:
            ChosenIndex = (GetRandomBool()) ? GetRandomInt(0, ActiveList.Num() - 1) : ActiveList.Num() - 1;
            break;
        }
        FIntPoint Current = ActiveList[ChosenIndex];

        TArray<FIntPoint> UnvisitedNeighbors;
        if (Current.X + 1 < GridWidth && !Grid[Current.Y][Current.X + 1].bVisited)
            UnvisitedNeighbors.Add(FIntPoint(Current.X + 1, Current.Y));
        if (Current.X > 0 && !Grid[Current.Y][Current.X - 1].bVisited)
            UnvisitedNeighbors.Add(FIntPoint(Current.X - 1, Current.Y));
        if (Current.Y + 1 < GridHeight && !Grid[Current.Y + 1][Current.X].bVisited)
            UnvisitedNeighbors.Add(FIntPoint(Current.X, Current.Y + 1));
        if (Current.Y > 0 && !Grid[Current.Y - 1][Current.X].bVisited)
            UnvisitedNeighbors.Add(FIntPoint(Current.X, Current.Y - 1));

        if (UnvisitedNeighbors.Num() > 0)
        {
            FIntPoint Next = UnvisitedNeighbors[GetRandomInt(0, UnvisitedNeighbors.Num() - 1)];
            if (Next.X > Current.X)       // Восток
                Grid[Current.Y][Current.X].IsEastWall = false;
            else if (Next.X < Current.X)  // Запад
                Grid[Current.Y][Next.X].IsEastWall = false;
            else if (Next.Y > Current.Y)  // Север
                Grid[Current.Y][Current.X].IsNorthWall = false;
            else if (Next.Y < Current.Y)  // Юг
                Grid[Next.Y][Current.X].IsNorthWall = false;

            Grid[Next.Y][Next.X].bVisited = true;
            ActiveList.Add(Next);
        }
        else
        {
            ActiveList.RemoveAt(ChosenIndex);
        }
    }
}

void AGrowingTreeMazeGenerator::ChooseEntranceAndExit()
{
    EntranceSide = EWallSide::South;
    EntranceCellIndex = GetRandomInt(0, GridWidth - 1);
    ExitSide = EWallSide::North;
    ExitCellIndex = GridWidth / 2;
    
    if (EntranceSide == EWallSide::South)
        Grid[0][EntranceCellIndex].bVisited = false;
    if (ExitSide == EWallSide::North)
        Grid[GridHeight - 1][ExitCellIndex].IsNorthWall = false;
}

FIntPoint AGrowingTreeMazeGenerator::GetStartCell() const
{
    return GetSideCoordinates(EntranceSide, EntranceCellIndex);
}

FIntPoint AGrowingTreeMazeGenerator::GetGoalCell() const
{
    return GetSideCoordinates(ExitSide, ExitCellIndex);
}

FIntPoint AGrowingTreeMazeGenerator::GetSideCoordinates(EWallSide Side, int32 CellIndex) const
{
    switch (Side)
    {
    case EWallSide::South: return FIntPoint(CellIndex, 0);
    case EWallSide::North: return FIntPoint(CellIndex, GridHeight - 1);
    case EWallSide::West:  return FIntPoint(0, CellIndex);
    case EWallSide::East:  return FIntPoint(GridWidth - 1, CellIndex);
    default: return FIntPoint::ZeroValue;
    }
}

bool AGrowingTreeMazeGenerator::EnsurePathfinder()
{
    if (!Pathfinder)
    {
        Pathfinder = NewObject<UMazePathfinder>(this);
        if (!Pathfinder) return false;
    }
    return true;
}

// Визуализация
void AGrowingTreeMazeGenerator::BuildMazeVisuals()
{
    BuildFloorMesh();
    BuildWalls();
}

void AGrowingTreeMazeGenerator::BuildFloorMesh()
{
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector2D> UVs;
    GenerateFloorMesh(Vertices, Triangles, UVs);
    FloorMeshComponent->CreateMeshSection(0, Vertices, Triangles, TArray<FVector>(), UVs,
        TArray<FColor>(), TArray<FProcMeshTangent>(), true);
    if (FloorMaterial) FloorMeshComponent->SetMaterial(0, FloorMaterial);
}

void AGrowingTreeMazeGenerator::GenerateFloorMesh(TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector2D>& UVs)
{
    float TotalWidth = GridWidth * CellSize;
    float TotalHeight = GridHeight * CellSize;
    Vertices.Add(FVector(0, 0, 0));
    Vertices.Add(FVector(TotalWidth, 0, 0));
    Vertices.Add(FVector(0, TotalHeight, 0));
    Vertices.Add(FVector(TotalWidth, TotalHeight, 0));
    Triangles.Add(0); Triangles.Add(2); Triangles.Add(1);
    Triangles.Add(1); Triangles.Add(2); Triangles.Add(3);
    UVs.Add(FVector2D(0, 0));
    UVs.Add(FVector2D(1, 0));
    UVs.Add(FVector2D(0, 1));
    UVs.Add(FVector2D(1, 1));
}

void AGrowingTreeMazeGenerator::BuildWalls()
{
    if (WallInstancedMeshComponent->GetStaticMesh() != WallMesh)
        WallInstancedMeshComponent->SetStaticMesh(WallMesh);
    if (WallMaterial)
    {
        for (int32 i = 0; i < WallInstancedMeshComponent->GetNumMaterials(); ++i)
            WallInstancedMeshComponent->SetMaterial(i, WallMaterial);
    }

    auto IsEntranceOrExit = [&](EWallSide Side, int32 Index) -> bool
    {
        return (Side == EntranceSide && Index == EntranceCellIndex) ||
               (Side == ExitSide     && Index == ExitCellIndex);
    };

    // Внутренние стены
    for (int32 Y = 0; Y < GridHeight; ++Y)
    {
        for (int32 X = 0; X < GridWidth; ++X)
        {
            const FCell& Cell = Grid[Y][X];
            FVector CellLocation(X * CellSize, Y * CellSize, 0.0f);
            if (Cell.IsEastWall)
            {
                FVector WallLocation = CellLocation + FVector(CellSize, CellSize * 0.5f, WallHeight * 0.5f);
                FTransform WallTransform(FRotator::ZeroRotator, WallLocation, FVector(WallThickness, CellSize, WallHeight) / 100.0f);
                WallInstancedMeshComponent->AddInstance(WallTransform);
            }
            if (Cell.IsNorthWall)
            {
                FVector WallLocation = CellLocation + FVector(CellSize * 0.5f, CellSize, WallHeight * 0.5f);
                FTransform WallTransform(FRotator(0, 90, 0), WallLocation, FVector(WallThickness, CellSize, WallHeight) / 100.0f);
                WallInstancedMeshComponent->AddInstance(WallTransform);
            }
        }
    }

    // Внешние стены
    for (int32 Y = 0; Y < GridHeight; ++Y)
    {
        if (IsEntranceOrExit(EWallSide::West, Y)) continue;
        FVector CellLocation(0, Y * CellSize, 0.0f);
        FVector WallLocation = CellLocation + FVector(0.0f, CellSize * 0.5f, WallHeight * 0.5f);
        FTransform WallTransform(FRotator::ZeroRotator, WallLocation, FVector(WallThickness, CellSize, WallHeight) / 100.0f);
        WallInstancedMeshComponent->AddInstance(WallTransform);
    }
    for (int32 X = 0; X < GridWidth; ++X)
    {
        if (IsEntranceOrExit(EWallSide::South, X)) continue;
        FVector CellLocation(X * CellSize, 0, 0.0f);
        FVector WallLocation = CellLocation + FVector(CellSize * 0.5f, 0.0f, WallHeight * 0.5f);
        FTransform WallTransform(FRotator(0, 90, 0), WallLocation, FVector(WallThickness, CellSize, WallHeight) / 100.0f);
        WallInstancedMeshComponent->AddInstance(WallTransform);
    }

    // Восточная внешняя стена
    for (int32 Y = 0; Y < GridHeight; ++Y)
    {
        if (IsEntranceOrExit(EWallSide::East, Y)) continue;
        if (!Grid[Y][GridWidth-1].IsEastWall)
        {
            FVector CellLocation((GridWidth-1) * CellSize, Y * CellSize, 0.0f);
            FVector WallLocation = CellLocation + FVector(CellSize, CellSize * 0.5f, WallHeight * 0.5f);
            FTransform WallTransform(FRotator::ZeroRotator, WallLocation, FVector(WallThickness, CellSize, WallHeight) / 100.0f);
            WallInstancedMeshComponent->AddInstance(WallTransform);
        }
    }

    // Северная внешняя стена
    for (int32 X = 0; X < GridWidth; ++X)
    {
        if (IsEntranceOrExit(EWallSide::North, X)) continue;
        if (!Grid[GridHeight-1][X].IsNorthWall)
        {
            FVector CellLocation(X * CellSize, (GridHeight-1) * CellSize, 0.0f);
            FVector WallLocation = CellLocation + FVector(CellSize * 0.5f, CellSize, WallHeight * 0.5f);
            FTransform WallTransform(FRotator(0, 90, 0), WallLocation, FVector(WallThickness, CellSize, WallHeight) / 100.0f);
            WallInstancedMeshComponent->AddInstance(WallTransform);
        }
    }
    UE_LOG(LogTemp, Log, TEXT("BuildWalls: Created %d wall instances."), WallInstancedMeshComponent->GetInstanceCount());
}

int32 AGrowingTreeMazeGenerator::GetRandomInt(int32 Min, int32 Max) { return RandomStream.RandRange(Min, Max); }
bool AGrowingTreeMazeGenerator::GetRandomBool() { return RandomStream.RandRange(0, 1) == 1; }