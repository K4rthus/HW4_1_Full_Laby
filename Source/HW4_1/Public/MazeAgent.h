#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MazeAgent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPathCompleted);

class UMazePathfinder;
class AGrowingTreeMazeGenerator;
struct FVisualStep;

UCLASS()
class HW4_1_API AMazeAgent : public APawn
{
	GENERATED_BODY()
public:
	AMazeAgent();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* VisualComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Speed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization")
	float VisualizationStepDelay = 0.0025f;

	UPROPERTY(BlueprintAssignable)
	FOnPathCompleted OnPathCompleted;

	UFUNCTION(BlueprintCallable)
	void SetPath(const TArray<FIntPoint>& NewPath, float CellSize);
	UFUNCTION(BlueprintCallable)
	void StopMovement();
	UFUNCTION(BlueprintCallable)
	void UpdateGoal(const FIntPoint& NewGoal, AGrowingTreeMazeGenerator* MazeGen);
	UFUNCTION(BlueprintCallable)
	FIntPoint GetCurrentCell(float CellSize) const;

	bool IsMoving() const { return bMoving; }
	
	void VisualizeSearchSteps(const TArray<FVisualStep>& Steps);

protected:
	virtual void Tick(float DeltaTime) override;

private:
	TArray<FVector> PathPoints;
	int32 CurrentTargetIndex = 0;
	bool bMoving = false;
	void MoveToNextTarget();
	
	FTimerHandle VisualizationTimer;
	TArray<FIntPoint> PendingPath;
	float PendingCellSize;
	TArray<FVisualStep> PendingSteps;
	int32 CurrentStepIndex = 0;
	void ProcessNextStep();
	void FinishVisualization(const TArray<FIntPoint>& Path, float CellSize);
};