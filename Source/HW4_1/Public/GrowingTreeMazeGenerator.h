#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "MazeCell.h"
#include "GrowingTreeMazeGenerator.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMazeRegenerated, AGrowingTreeMazeGenerator*);

UENUM()
enum class EWallSide : uint8 { South, North, West, East };

class UMazePathfinder;

UCLASS(Blueprintable, BlueprintType)
class HW4_1_API AGrowingTreeMazeGenerator : public AActor
{
	GENERATED_BODY()

public:
	AGrowingTreeMazeGenerator();

	FOnMazeRegenerated OnMazeRegenerated;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Settings")
	int32 GridWidth = 21;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Settings")
	int32 GridHeight = 21;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Settings")
	float CellSize = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Settings")
	float WallHeight = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Settings")
	float WallThickness = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Settings")
	int32 RandomSeed = 12345;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Settings")
	int32 GrowingTreeStrategy = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Visuals")
	UStaticMesh* WallMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Visuals")
	UMaterialInterface* WallMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Visuals")
	UMaterialInterface* FloorMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Pathfinding")
	bool bVisualizePath = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze|Pathfinding")
	float PathDebugDuration = 10000.0f;

	UFUNCTION(BlueprintCallable, Category = "Maze")
	void GenerateMaze();

	UFUNCTION(BlueprintCallable, Category = "Maze")
	void ClearMaze();

	UFUNCTION(BlueprintCallable, Category = "Maze")
	FIntPoint GetStartCell() const;

	UFUNCTION(BlueprintCallable, Category = "Maze")
	FIntPoint GetGoalCell() const;

	UMazePathfinder* GetPathfinder() const { return Pathfinder; }

	float GetCellSize() const { return CellSize; }

private:
	TArray<TArray<FCell>> Grid;
	FRandomStream RandomStream;

	UPROPERTY()
	UProceduralMeshComponent* FloorMeshComponent;

	UPROPERTY()
	UInstancedStaticMeshComponent* WallInstancedMeshComponent;

	UPROPERTY()
	USceneComponent* RootSceneComponent;

	EWallSide EntranceSide;
	int32 EntranceCellIndex;
	EWallSide ExitSide;
	int32 ExitCellIndex;

	UPROPERTY()
	UMazePathfinder* Pathfinder;

	void RunGrowingTreeAlgorithm();
	void ChooseEntranceAndExit();
	bool EnsurePathfinder();
	FIntPoint GetSideCoordinates(EWallSide Side, int32 CellIndex) const;

	int32 GetRandomInt(int32 Min, int32 Max);
	bool GetRandomBool();

	void BuildMazeVisuals();
	void BuildFloorMesh();
	void BuildWalls();
	void GenerateFloorMesh(TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector2D>& UVs);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};