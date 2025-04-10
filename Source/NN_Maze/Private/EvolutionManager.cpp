#include "EvolutionManager.h"
#include "Math/UnrealMathUtility.h"

UEvolutionManager::UEvolutionManager()
{
    // Default: 5 groups
    NumGroups = 5;

    // Default mutation rates for each group. (The array should have NumGroups elements.)
    MutationRates = { 0.1f, 0.3f, 0.5f, 2.0f, 9.0f };
}

void UEvolutionManager::ProcessGeneration(TArray<UNeuralNetwork*>& CurrentGeneration, TArray<UNeuralNetwork*>& NextGeneration, float& OutGenerationFitnessMean, int32 PopulationSize)
{
    // Calculate average fitness for the current generation.
    OutGenerationFitnessMean = 0.f;
    for (int32 i = 0; i < PopulationSize; i++)
    {
        if (CurrentGeneration.IsValidIndex(i) && CurrentGeneration[i])
        {
            OutGenerationFitnessMean += CurrentGeneration[i]->Fitness;
        }
    }
    OutGenerationFitnessMean /= PopulationSize;

    // Sort the current generation networks by fitness (highest first).
    CurrentGeneration.Sort([](const UNeuralNetwork& A, const UNeuralNetwork& B)
        {
            return A.Fitness > B.Fitness;
        });

    // Validate NumGroups.
    if (NumGroups <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid NumGroups: %d. Defaulting to 1 group."), NumGroups);
        NumGroups = 1;
    }

    // Ensure that the MutationRates array matches the number of groups.
    if (MutationRates.Num() != NumGroups)
    {
        UE_LOG(LogTemp, Warning, TEXT("MutationRates array size (%d) does not match NumGroups (%d). Using default mutation rate (0.1f) for all groups."), MutationRates.Num(), NumGroups);
        MutationRates.Empty();
        for (int32 i = 0; i < NumGroups; i++)
        {
            MutationRates.Add(0.1f);
        }
    }

    // Distribute PopulationSize across groups as evenly as possible.
    int32 BaseGroupSize = PopulationSize / NumGroups;
    int32 Remainder = PopulationSize % NumGroups;

    NextGeneration.Empty();

    int32 startIndex = 0;
    for (int32 group = 0; group < NumGroups; group++)
    {
        // Si Remainder > 0, ajoutez 1 au groupe courant.
        int32 currentGroupSize = BaseGroupSize + (group < Remainder ? 1 : 0);
        for (int32 j = 0; j < currentGroupSize; j++)
        {
            int32 netIndex = startIndex + j;
            if (CurrentGeneration.IsValidIndex(netIndex) && CurrentGeneration[netIndex])
            {
                UNeuralNetwork* NewNet = NewObject<UNeuralNetwork>(this, UNeuralNetwork::StaticClass());
                TArray<int32> LayerConfig = CurrentGeneration[netIndex]->LayerSizes; // Réutilise la configuration existante.
                NewNet->Initialize(LayerConfig);
                NewNet->CopyWeights(CurrentGeneration[netIndex]);

                float MutationRate = MutationRates[group];
                NewNet->Mutate(MutationRate);
                NextGeneration.Add(NewNet);
            }
        }
        startIndex += currentGroupSize;
    }
}
