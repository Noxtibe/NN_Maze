#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MazeAgent.h"
#include "NeuralNetwork.h"
#include "MazeManager.generated.h"

UCLASS()
class NN_MAZE_API AMazeManager : public AActor
{
	GENERATED_BODY()
	
public:

    AMazeManager();

protected:

    virtual void BeginPlay() override;

public:

    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, Category = "Agent")
        TSubclassOf<AMazeAgent> AgentBlueprint;

    UPROPERTY(EditAnywhere, Category = "Agent")
        FVector StartPosition;

    UPROPERTY(EditAnywhere, Category = "Agent")
        int32 PopulationSize;

    UPROPERTY(EditAnywhere, Category = "Agent")
        float TimeLimit;

private:

    int32 GenerationCount;
    bool bIsTraining;
    TArray<UNeuralNetwork*> CurrentGeneration;
    TArray<UNeuralNetwork*> NextGeneration;
    TArray<AMazeAgent*> Agents;
    float GenerationFitnessMean;

    float TotalSimulationTime;
    int32 TotalSimulations;

    void CloseTimer();
    void InitAgentNetworks();
    void CreateAgents();

    FTimerHandle TimerHandle_CloseTimer;
};
