#pragma once
#include "CoreMinimal.h"
#include "MazeCell.generated.h"

USTRUCT(BlueprintType)
struct HW4_1_API FCell
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze")
	bool IsEastWall = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze")
	bool IsNorthWall = true;
	
	bool bVisited = false;

	FCell() = default;
};