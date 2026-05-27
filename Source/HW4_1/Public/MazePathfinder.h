#pragma once
#include "CoreMinimal.h"
#include "MazeCell.h"
#include "MazePathfinder.generated.h"

UENUM()
enum class EVisStepType : uint8
{
	OpenSetAdd,
	ClosedSetAdd
};

USTRUCT(BlueprintType)
struct FVisualStep
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)
	FIntPoint Cell;
	UPROPERTY(BlueprintReadOnly)
	EVisStepType StepType;
};

USTRUCT()
struct FPathNode
{
	GENERATED_BODY()
	int32 X = 0;
	int32 Y = 0;
	bool bWalkable = true;
	float Cost = FLT_MAX;
	int32 PrevIndex = -1;
};

UCLASS(BlueprintType)
class HW4_1_API UMazePathfinder : public UObject
{
	GENERATED_BODY()
public:
	void Initialize(int32 InGridWidth, int32 InGridHeight, float InCellSize, float InWallHeight,
					const TArray<TArray<FCell>>& InGrid);
	
	bool FindPath(FIntPoint Start, FIntPoint End, TArray<FIntPoint>& OutPath,
				  TArray<FVisualStep>* OutSteps = nullptr);
	void VisualizePath(const TArray<FIntPoint>& Path, float Duration = 10000.0f, bool bPersistent = true) const;
	FVector GetCellCenter(int32 X, int32 Y) const;
	bool IsCellWalkable(int32 X, int32 Y) const { return Nodes[GetIndex(X, Y)].bWalkable; }

private:
	int32 GridWidth = 0;
	int32 GridHeight = 0;
	float CellSize = 200.0f;
	float WallHeight = 100.0f;
	TArray<FPathNode> Nodes;
	const TArray<TArray<FCell>>* MazeGrid = nullptr;

	int32 GetIndex(int32 X, int32 Y) const { return Y * GridWidth + X; }
	bool IsValidCoord(int32 X, int32 Y) const { return X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight; }
	void GetNeighbors(int32 X, int32 Y, TArray<int32>& OutNeighbors) const;
};
