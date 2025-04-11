#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NeuralNetwork.h"
#include "Checkpoint.h"
#include "MazeAgent.generated.h"

UCLASS()
class NN_MAZE_API AMazeAgent : public ACharacter
{
    GENERATED_BODY()

public:
    AMazeAgent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Performs raycasts in various directions to collect sensor data
    void RaycastVision();

    // Processes neural network input and updates movement based on network output
    void ProcessNeuralNetwork();

    // Movement properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RotationSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed;

    // Vision property
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vision")
    float MaxViewDistance;

    // Learning parameters (rewards and penalties)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
    float FitnessTimeDecreaseRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
    float FitnessCheckpointIncreaseRate;

    UPROPERTY(BlueprintReadWrite, Category = "Learning")
    float Fitness;

    UPROPERTY(BlueprintReadWrite, Category = "Learning")
    bool IsActive;

    // Smoothing factor for sensor readings (0 to 1). Lower value means more smoothing.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    float SensorSmoothingFactor;

    // Sensor values from raycasts
    UPROPERTY(BlueprintReadWrite, Category = "Input")
    float DistForward;
    UPROPERTY(BlueprintReadWrite, Category = "Input")
    float DistLeft;
    UPROPERTY(BlueprintReadWrite, Category = "Input")
    float DistDiagLeft;
    UPROPERTY(BlueprintReadWrite, Category = "Input")
    float DistRight;
    UPROPERTY(BlueprintReadWrite, Category = "Input")
    float DistDiagRight;

    // Relative information to the exit.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    FVector ExitLocation;  // Must be set externally (e.g., by MazeManager)

    UPROPERTY(BlueprintReadWrite, Category = "Input")
    float RelativeAngleToExit;  // Normalized to [-1, 1]

    UPROPERTY(BlueprintReadWrite, Category = "Input")
    float NormalizedDistanceToExit;  // Normalized distance (0 to 1)

    // Neural network associated with this agent
    UPROPERTY(BlueprintReadWrite, Category = "AI")
    UNeuralNetwork* NeuralNet;

public:
    FVector LastPosition;
    float DistanceTraveled;

    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    UFUNCTION()
    void OnBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

private:
    // Variables for optimizing logs and raycasts
    float LastLogTime;           // Time of last log output
    float LastRaycastUpdateTime; // Time of last raycast execution
    float RaycastUpdateInterval; // Minimum interval between raycasts (e.g., 0.1 sec)

    // Previous sensor values for hysteresis/smoothing
    float PrevDistForward;
    float PrevDistLeft;
    float PrevDistDiagLeft;
    float PrevDistRight;
    float PrevDistDiagRight;

    // --- New dedicated functions for fitness modification ---

    // Applies a reward based on the distance traveled.
    void ApplyDistanceReward(float DeltaDistance);

    // Applies a penalty based on the passage of time.
    void ApplyTimePenalty(float DeltaTime);

    // Applies a penalty for hitting a wall.
    void ApplyWallPenalty();

    // Applies a reward for reaching a checkpoint.
    void ApplyCheckpointReward(float RewardMultiplier);
};
