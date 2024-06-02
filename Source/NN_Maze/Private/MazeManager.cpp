#include "MazeManager.h"
#include "MazeAgent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AMazeManager::AMazeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    StartPosition = FVector(20.f, 0.f, -20.f);
    PopulationSize = 1; // Utilisation d'un seul agent pour la simulation
    TimeLimit = 30.f; // Ajustez selon vos besoins, ici on allonge la génération
    GenerationCount = 0;
    bIsTraining = false;
    GenerationFitnessMean = 0.f;
    TotalSimulationTime = 0.f;
    TotalSimulations = 0;
}

void AMazeManager::BeginPlay()
{
    Super::BeginPlay();
    InitAgentNetworks();
    CreateAgents();
    GetWorld()->GetTimerManager().SetTimer(TimerHandle_CloseTimer, this, &AMazeManager::CloseTimer, TimeLimit, false);
    bIsTraining = true;
}

void AMazeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsTraining)
    {
        TotalSimulationTime += DeltaTime;
    }

    FString DebugMessage = FString::Printf(TEXT("Total Simulation Time: %.2f seconds\nTotal Simulations: %d\nCurrent Generation: %d"), TotalSimulationTime, TotalSimulations, GenerationCount);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Yellow, DebugMessage);
    }

    if (!bIsTraining)
    {
        ProcessGeneration();
    }

    UpdateAgents(DeltaTime);
}

void AMazeManager::CloseTimer()
{
    GenerationCount++;
    TotalSimulations += PopulationSize;
    bIsTraining = false;
}

void AMazeManager::InitAgentNetworks()
{
    CurrentGeneration.Empty();
    for (int32 i = 0; i < PopulationSize; i++)
    {
        UNeuralNetwork* Net = NewObject<UNeuralNetwork>(this, UNeuralNetwork::StaticClass());
        TArray<int32> LayerConfig = { 6, 8, 10, 6, 2 }; // Exemple de layers
        Net->Initialize(LayerConfig);
        Net->Mutate(0.5f);
        CurrentGeneration.Add(Net);
    }
}

void AMazeManager::CreateAgents()
{
    for (AMazeAgent* Agent : Agents)
    {
        if (Agent)
        {
            Agent->Destroy();
        }
    }
    Agents.Empty();

    for (int32 i = 0; i < PopulationSize; i++)
    {
        if (AgentBlueprint)
        {
            FVector SpawnLocation = StartPosition; // Utilisation de la même position de spawn
            FRotator SpawnRotation = FRotator::ZeroRotator;
            AMazeAgent* Agent = GetWorld()->SpawnActor<AMazeAgent>(AgentBlueprint, SpawnLocation, SpawnRotation);
            if (Agent)
            {
                // Associez le réseau de neurones à l'agent
                if (CurrentGeneration.IsValidIndex(i))
                {
                    Agent->Outputs = TArray<float>(); // Réinitialiser les sorties pour le nouvel agent
                }
                Agents.Add(Agent);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to spawn agent %d"), i);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("AgentBlueprint is not set."));
        }
    }
}

void AMazeManager::UpdateAgents(float DeltaTime)
{
    for (int32 i = 0; i < PopulationSize; i++)
    {
        if (Agents.IsValidIndex(i) && CurrentGeneration.IsValidIndex(i))
        {
            AMazeAgent* Agent = Agents[i];
            UNeuralNetwork* Network = CurrentGeneration[i];
            if (Agent && Network && Network->LayerSizes.Num() > 0)
            {
                float Vel = Agent->Speed / Agent->MaxViewDistance;
                float DistForward = Agent->DistForward / Agent->MaxViewDistance;
                float DistLeft = Agent->DistLeft / Agent->MaxViewDistance;
                float DistDiagLeft = Agent->DistDiagLeft / Agent->MaxViewDistance;
                float DistRight = Agent->DistRight / Agent->MaxViewDistance;
                float DistDiagRight = Agent->DistDiagRight / Agent->MaxViewDistance;

                TArray<float> Inputs = { Vel, DistForward, DistLeft, DistDiagLeft, DistRight, DistDiagRight };

                
                
                // Ajout de messages de debug pour les entrées du réseau de neurones
                UE_LOG(LogTemp, Log, TEXT("Agent %d Inputs: Vel=%.2f, DistForward=%.2f, DistLeft=%.2f, DistDiagLeft=%.2f, DistRight=%.2f, DistDiagRight=%.2f"),
                    i, Vel, DistForward, DistLeft, DistDiagLeft, DistRight, DistDiagRight);

                // Vérifiez que Network n'est pas null avant d'appeler GetInputSize()
                if (Inputs.Num() == Network->GetInputSize())
                {
                    TArray<float> Outputs = Network->FeedForward(Inputs);

                    // Ajout de messages de debug pour les sorties du réseau de neurones
                    if (Outputs.Num() >= 2)
                    {
                        UE_LOG(LogTemp, Log, TEXT("Agent %d Outputs: Speed=%.2f, Rotation=%.2f"), i, Outputs[0], Outputs[1]);
                    }

                    Agent->Outputs = Outputs;

                    // Ajout de messages de debug pour le mouvement de l'agent
                    UE_LOG(LogTemp, Log, TEXT("Agent %d MoveDir: %.2f, Speed: %.2f"), i, Agent->MoveDir.Size(), Agent->Speed);
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Invalid input size for neural network. Expected: %d, Got: %d"), Network->GetInputSize(), Inputs.Num());
                }
            }
        }
    }
}

void AMazeManager::ProcessGeneration()
{
    if (GenerationCount == 0)
    {
        InitAgentNetworks();
        CreateAgents();
        GetWorld()->GetTimerManager().SetTimer(TimerHandle_CloseTimer, this, &AMazeManager::CloseTimer, TimeLimit, false);
        bIsTraining = true;
    }
    else
    {
        GenerationFitnessMean = 0.f;
        for (int32 i = 0; i < PopulationSize; i++)
        {
            if (Agents.IsValidIndex(i) && CurrentGeneration.IsValidIndex(i))
            {
                AMazeAgent* Agent = Agents[i];
                if (Agent)
                {
                    float Fitness = Agent->Fitness;
                    if (CurrentGeneration[i])
                    {
                        CurrentGeneration[i]->Fitness = Fitness;
                        GenerationFitnessMean += CurrentGeneration[i]->Fitness;
                    }
                }
            }
        }
        GenerationFitnessMean /= PopulationSize;

        CurrentGeneration.Sort([](const UNeuralNetwork& A, const UNeuralNetwork& B) {
            return A.Fitness > B.Fitness;
            });

        NextGeneration.Empty();
        for (int32 i = 0; i < 5; i++)
        {
            for (int32 j = 0; j < PopulationSize / 5; j++)
            {
                if (CurrentGeneration.IsValidIndex(j) && CurrentGeneration[j])
                {
                    UNeuralNetwork* Network = NewObject<UNeuralNetwork>(this, UNeuralNetwork::StaticClass());
                    TArray<int32> LayerConfig = CurrentGeneration[j]->LayerSizes; // Utiliser la même configuration de couches
                    Network->Initialize(LayerConfig);
                    Network->CopyWeights(CurrentGeneration[j]);

                    float MutationRate = 0.1f;
                    switch (i)
                    {
                    case 0:
                        MutationRate = 0.1f;
                        break;
                    case 1:
                        MutationRate = 0.3f;
                        break;
                    case 2:
                        MutationRate = 0.5f;
                        break;
                    case 3:
                        MutationRate = 2.f;
                        break;
                    case 4:
                        MutationRate = 9.f;
                        break;
                    }
                    Network->Mutate(MutationRate);
                    NextGeneration.Add(Network);
                }
            }
        }

        // Ajout de vérifications de validité des réseaux avant de remplacer la génération actuelle
        for (UNeuralNetwork* Network : NextGeneration)
        {
            if (!Network || Network->LayerSizes.Num() == 0)
            {
                UE_LOG(LogTemp, Error, TEXT("Invalid network in NextGeneration."));
                return;
            }
        }

        CurrentGeneration = NextGeneration;
        CreateAgents();
        GetWorld()->GetTimerManager().SetTimer(TimerHandle_CloseTimer, this, &AMazeManager::CloseTimer, TimeLimit, false);
        bIsTraining = true;
    }
}
