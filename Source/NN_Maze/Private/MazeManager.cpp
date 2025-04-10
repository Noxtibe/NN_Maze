#include "MazeManager.h"
#include "MazeAgent.h"
#include "NeuralNetwork.h"
#include "EvolutionManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AMazeManager::AMazeManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default property initialization
    StartPosition = FVector(0.0f, 0.0f, 0.0f);
    PopulationSize = 5;  // Ensure this is set greater than 0 in the editor
    TimeLimit = 10.f;    // Generation duration in seconds
    GenerationCount = 0;
    bIsTraining = false;
    GenerationFitnessMean = 0.f;
    TotalSimulationTime = 0.f;
    TotalSimulations = 0;
}

void AMazeManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("MazeManager BeginPlay: Starting Generation %d"), GenerationCount);

    // Initialize neural networks for the current generation
    InitAgentNetworks();
    // Create agents and assign them their neural networks
    CreateAgents();
    // Set the timer to end the generation
    GetWorld()->GetTimerManager().SetTimer(TimerHandle_CloseTimer, this, &AMazeManager::CloseTimer, TimeLimit, false);
    bIsTraining = true;

    EvolutionManager = NewObject<UEvolutionManager>(this, UEvolutionManager::StaticClass());
}

void AMazeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsTraining)
    {
        TotalSimulationTime += DeltaTime;
    }

    // Display debug information on screen
    FString DebugMessage = FString::Printf(TEXT("Simulation Time: %.2f sec, Total Simulations: %d, Generation: %d"),
        TotalSimulationTime, TotalSimulations, GenerationCount);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Yellow, DebugMessage);
    }

    // If training time is over, process the evolution cycle
    if (!bIsTraining)
    {
        ProcessGeneration();
    }

    // Optionally update agents—each agent’s own Tick handles its neural network processing
    UpdateAgents(DeltaTime);
}

void AMazeManager::CloseTimer()
{
    GenerationCount++;
    TotalSimulations += PopulationSize;
    bIsTraining = false;
    UE_LOG(LogTemp, Log, TEXT("Generation %d complete. Total simulations: %d"), GenerationCount, TotalSimulations);
}

void AMazeManager::InitAgentNetworks()
{
    // Clear any previous generation networks.
    CurrentGeneration.Empty();

    // Use the editable network configuration; if empty, use a default.
    TArray<int32> LayerConfig = NetworkLayerConfiguration;
    if (LayerConfig.Num() == 0)
    {
        LayerConfig = { 6, 8, 10, 6, 2 };
        UE_LOG(LogTemp, Warning, TEXT("NetworkLayerConfiguration is empty. Using default configuration."));
    }

    // Create a new neural network for each agent in the population.
    for (int32 i = 0; i < PopulationSize; i++)
    {
        UNeuralNetwork* Net = NewObject<UNeuralNetwork>(this, UNeuralNetwork::StaticClass());
        if (!Net)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create neural network for agent %d"), i);
            continue;
        }
        Net->Initialize(LayerConfig);
        // Apply an initial mutation for diversity
        Net->Mutate(0.5f);
        CurrentGeneration.Add(Net);
    }
    UE_LOG(LogTemp, Log, TEXT("Initialized %d neural networks for the current generation."), CurrentGeneration.Num());
}

void AMazeManager::CreateAgents()
{
    UE_LOG(LogTemp, Log, TEXT("CreateAgents() called. PopulationSize: %d"), PopulationSize);

    // Safely destroy existing agents.
    for (AMazeAgent* Agent : Agents)
    {
        if (Agent && IsValid(Agent))
        {
            Agent->Destroy();
        }
    }
    Agents.Empty();

    // Check that the AgentBlueprint is set
    if (!AgentBlueprint)
    {
        UE_LOG(LogTemp, Error, TEXT("AgentBlueprint is not set! Please assign a valid MazeAgent Blueprint in MazeManager."));
        return;
    }

    // Spawn new agents and assign each its corresponding neural network.
    for (int32 i = 0; i < PopulationSize; i++)
    {
        FVector SpawnLocation = StartPosition;
        FRotator SpawnRotation = FRotator::ZeroRotator;
        UE_LOG(LogTemp, Log, TEXT("Spawning Agent %d at location %s"), i, *SpawnLocation.ToString());

        // Spawn the agent safely
        AMazeAgent* NewAgent = GetWorld()->SpawnActor<AMazeAgent>(AgentBlueprint, SpawnLocation, SpawnRotation);
        if (!NewAgent)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn agent %d"), i);
            continue;
        }

        // Verify that a neural network exists for this index; if not, log warning.
        if (CurrentGeneration.IsValidIndex(i) && CurrentGeneration[i])
        {
            NewAgent->NeuralNet = CurrentGeneration[i];
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("CurrentGeneration does not have a valid neural network at index %d"), i);
            NewAgent->NeuralNet = nullptr;
        }
        Agents.Add(NewAgent);
        UE_LOG(LogTemp, Log, TEXT("Agent %d spawned successfully."), i);
    }
}

void AMazeManager::UpdateAgents(float DeltaTime)
{
    // Agents are handling their own updates in their Tick() functions.
}

void AMazeManager::ProcessGeneration()
{
    UE_LOG(LogTemp, Log, TEXT("Processing Generation %d"), GenerationCount);

    // (Optional) Update the fitness values from agents to the respective neural networks.
    // For each agent, assign its fitness to its network.
    for (int32 i = 0; i < PopulationSize; i++)
    {
        if (Agents.IsValidIndex(i))
        {
            AMazeAgent* Agent = Agents[i];
            if (Agent && CurrentGeneration.IsValidIndex(i) && CurrentGeneration[i])
            {
                CurrentGeneration[i]->Fitness = Agent->Fitness;
            }
        }
    }

    // Delegate the evolution processing to the evolution manager.
    float NewGenerationAverageFitness = 0.f;
    if (EvolutionManager)
    {
        EvolutionManager->ProcessGeneration(CurrentGeneration, NextGeneration, NewGenerationAverageFitness, PopulationSize);
    }

    UE_LOG(LogTemp, Log, TEXT("Average fitness for Generation %d: %.2f"), GenerationCount, NewGenerationAverageFitness);

    // Replace the current generation with the new generation
    CurrentGeneration = NextGeneration;

    // Spawn new agents for the new generation
    CreateAgents();

    // Restart the generation timer and resume training
    GetWorld()->GetTimerManager().SetTimer(TimerHandle_CloseTimer, this, &AMazeManager::CloseTimer, TimeLimit, false);
    bIsTraining = true;
}
