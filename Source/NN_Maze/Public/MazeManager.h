#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MazeAgent.h"
#include "MazeManager.generated.h"

class UNeuralNetwork;
class UEvolutionManager;

UCLASS()
class NN_MAZE_API AMazeManager : public AActor
{
    GENERATED_BODY()

public:
    AMazeManager();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Agent blueprint (set this in the editor)
    UPROPERTY(EditAnywhere, Category = "Agent")
    TSubclassOf<AMazeAgent> AgentBlueprint;

    // Starting position for agents
    UPROPERTY(EditAnywhere, Category = "Agent")
    FVector StartPosition;

    // Number of agents in the simulation
    UPROPERTY(EditAnywhere, Category = "Agent")
    int32 PopulationSize;

    // Duration (in seconds) for each generation
    UPROPERTY(EditAnywhere, Category = "Agent")
    float TimeLimit;

    UPROPERTY()
    UEvolutionManager* EvolutionManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
    TArray<int32> NetworkLayerConfiguration;

private:
    // Evolution cycle functions
    void CloseTimer();
    void InitAgentNetworks();
    void CreateAgents();
    void UpdateAgents(float DeltaTime);
    void ProcessGeneration();

private:

    UPROPERTY()
    TArray<UNeuralNetwork*> CurrentGeneration;

    UPROPERTY()
    TArray<UNeuralNetwork*> NextGeneration;

    UPROPERTY()
    TArray<AMazeAgent*> Agents;

    int32 GenerationCount;
    bool bIsTraining;
    float GenerationFitnessMean;
    float TotalSimulationTime;
    int32 TotalSimulations;

    // Timer handle for generation end
    FTimerHandle TimerHandle_CloseTimer;
};
