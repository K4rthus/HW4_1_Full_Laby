#include "MazePathfinder.h"
#include "DrawDebugHelpers.h"

void UMazePathfinder::Initialize(int32 InGridWidth, int32 InGridHeight, float InCellSize, float InWallHeight,
                                 const TArray<TArray<FCell>>& InGrid)
{
    GridWidth = InGridWidth;
    GridHeight = InGridHeight;
    CellSize = InCellSize;
    WallHeight = InWallHeight;
    MazeGrid = &InGrid;

    Nodes.Empty();
    Nodes.SetNum(GridWidth * GridHeight);
    for (int32 Y = 0; Y < GridHeight; ++Y)
        for (int32 X = 0; X < GridWidth; ++X)
        {
            int32 Idx = GetIndex(X, Y);
            Nodes[Idx].X = X;
            Nodes[Idx].Y = Y;
            Nodes[Idx].bWalkable = true;
        }
}

void UMazePathfinder::GetNeighbors(int32 X, int32 Y, TArray<int32>& OutNeighbors) const
{
    OutNeighbors.Empty();
    if (!MazeGrid) return;
    const FCell& Cell = (*MazeGrid)[Y][X];

    if (X + 1 < GridWidth && !Cell.IsEastWall)
        OutNeighbors.Add(GetIndex(X + 1, Y));
    if (X > 0 && !(*MazeGrid)[Y][X - 1].IsEastWall)
        OutNeighbors.Add(GetIndex(X - 1, Y));
    if (Y + 1 < GridHeight && !Cell.IsNorthWall)
        OutNeighbors.Add(GetIndex(X, Y + 1));
    if (Y > 0 && !(*MazeGrid)[Y - 1][X].IsNorthWall)
        OutNeighbors.Add(GetIndex(X, Y - 1));
}

bool UMazePathfinder::FindPath(FIntPoint Start, FIntPoint End, TArray<FIntPoint>& OutPath,
                               TArray<FVisualStep>* OutSteps)
{
    OutPath.Empty();
    if (!IsValidCoord(Start.X, Start.Y) || !IsValidCoord(End.X, End.Y))
        return false;

    TArray<FVisualStep> LocalSteps;

    for (FPathNode& Node : Nodes)
    {
        Node.Cost = FLT_MAX;
        Node.PrevIndex = -1;
    }

    TArray<int32> OpenSet;
    TArray<bool> Visited;
    Visited.Init(false, Nodes.Num());

    int32 StartIdx = GetIndex(Start.X, Start.Y);
    int32 EndIdx = GetIndex(End.X, End.Y);
    Nodes[StartIdx].Cost = 0;
    OpenSet.Add(StartIdx);

    if (OutSteps)
        LocalSteps.Add({Start, EVisStepType::OpenSetAdd});

    while (OpenSet.Num() > 0)
    {
        int32 MinIdx = 0;
        for (int32 i = 1; i < OpenSet.Num(); ++i)
            if (Nodes[OpenSet[i]].Cost < Nodes[OpenSet[MinIdx]].Cost)
                MinIdx = i;

        int32 CurrentIdx = OpenSet[MinIdx];
        OpenSet.RemoveAt(MinIdx);
        Visited[CurrentIdx] = true;

        if (OutSteps)
            LocalSteps.Add({FIntPoint(Nodes[CurrentIdx].X, Nodes[CurrentIdx].Y), EVisStepType::ClosedSetAdd});

        if (CurrentIdx == EndIdx)
            break;

        int32 X = Nodes[CurrentIdx].X;
        int32 Y = Nodes[CurrentIdx].Y;
        TArray<int32> Neighbors;
        GetNeighbors(X, Y, Neighbors);

        for (int32 NeighborIdx : Neighbors)
        {
            if (Visited[NeighborIdx])
                continue;

            float NewCost = Nodes[CurrentIdx].Cost + 1.0f;
            if (NewCost < Nodes[NeighborIdx].Cost)
            {
                Nodes[NeighborIdx].Cost = NewCost;
                Nodes[NeighborIdx].PrevIndex = CurrentIdx;
                if (!OpenSet.Contains(NeighborIdx))
                {
                    OpenSet.Add(NeighborIdx);
                    if (OutSteps)
                        LocalSteps.Add({FIntPoint(Nodes[NeighborIdx].X, Nodes[NeighborIdx].Y), EVisStepType::OpenSetAdd});
                }
            }
        }
    }

    if (OutSteps)
        *OutSteps = MoveTemp(LocalSteps);

    if (Visited[EndIdx])
    {
        int32 Cur = EndIdx;
        while (Cur != -1)
        {
            OutPath.Insert(FIntPoint(Nodes[Cur].X, Nodes[Cur].Y), 0);
            Cur = Nodes[Cur].PrevIndex;
        }
        return true;
    }
    return false;
}

FVector UMazePathfinder::GetCellCenter(int32 X, int32 Y) const
{
    return FVector((X + 0.5f) * CellSize, (Y + 0.5f) * CellSize, WallHeight * 0.5f);
}

void UMazePathfinder::VisualizePath(const TArray<FIntPoint>& Path, float Duration, bool bPersistent) const
{
    UWorld* World = GetWorld();
    if (!World) return;
    const float DrawHeight = WallHeight + 20.0f;

    for (const FIntPoint& Pt : Path)
    {
        FVector Center = GetCellCenter(Pt.X, Pt.Y);
        Center.Z = DrawHeight;
        DrawDebugSphere(World, Center, CellSize * 0.15f, 8, FColor::Green, bPersistent, Duration, 0, 2.0f);
    }
    for (int32 i = 0; i < Path.Num() - 1; ++i)
    {
        FVector A = GetCellCenter(Path[i].X, Path[i].Y);
        FVector B = GetCellCenter(Path[i + 1].X, Path[i + 1].Y);
        A.Z = B.Z = DrawHeight;
        DrawDebugLine(World, A, B, FColor::Green, bPersistent, Duration, 0, 5.0f);
    }
}