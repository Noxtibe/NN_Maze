#include "EvolutionManager.h"
#include "Math/UnrealMathUtility.h"

UEvolutionManager::UEvolutionManager()
{
    // Set default evolutionary parameter values
    ElitismRate = 0.1f;             // 10% of the population is preserved (elitism)
    BaseMutationRate = 0.5f;        // Base mutation rate for offspring
    CrossoverProbability = 0.5f;    // 50% chance to take gene from parent1 in crossover
    TargetFitnessDifference = 10.f; // Target difference for dynamic mutation adaptation
}

void UEvolutionManager::ProcessGeneration(TArray<UNeuralNetwork*>& CurrentGeneration,
    TArray<UNeuralNetwork*>& NextGeneration,
    float& OutGenerationFitnessMean,
    int32 PopulationSize)
{
    // Calculate the average fitness for the current generation.
    OutGenerationFitnessMean = 0.f;
    for (int32 i = 0; i < PopulationSize; i++)
    {
        if (CurrentGeneration.IsValidIndex(i) && CurrentGeneration[i])
        {
            OutGenerationFitnessMean += CurrentGeneration[i]->Fitness;
        }
    }
    OutGenerationFitnessMean /= PopulationSize;

    // Sort the current generation by fitness in descending order (best networks first).
    CurrentGeneration.Sort([](const UNeuralNetwork& A, const UNeuralNetwork& B)
        {
            return A.Fitness > B.Fitness;
        });

    // Determine the number of elite networks to preserve.
    int32 ElitismCount = FMath::CeilToInt(PopulationSize * ElitismRate);
    if (ElitismCount < 1)
    {
        ElitismCount = 1;
    }

    // Clear the NextGeneration array.
    NextGeneration.Empty();

    // 1. Elitism: Clone and copy the top elite networks directly (without mutation).
    for (int32 i = 0; i < ElitismCount; i++)
    {
        UNeuralNetwork* EliteClone = NewObject<UNeuralNetwork>(this, UNeuralNetwork::StaticClass());
        EliteClone->Initialize(CurrentGeneration[i]->LayerSizes);
        EliteClone->CopyWeights(CurrentGeneration[i]);
        // Do not apply mutation to elite clones.
        NextGeneration.Add(EliteClone);
    }

    // 2. Generate offspring for the remainder of the population using crossover.
    int32 OffspringCount = PopulationSize - ElitismCount;
    // Use the top half of the population as the pool for parents.
    int32 ParentPoolSize = FMath::Max(1, PopulationSize / 2);

    for (int32 i = 0; i < OffspringCount; i++)
    {
        // Randomly select two parents from the top half of the sorted population.
        int32 ParentIndex1 = FMath::RandRange(0, ParentPoolSize - 1);
        int32 ParentIndex2 = FMath::RandRange(0, ParentPoolSize - 1);
        UNeuralNetwork* Parent1 = CurrentGeneration[ParentIndex1];
        UNeuralNetwork* Parent2 = CurrentGeneration[ParentIndex2];

        // Create a new network for the child.
        UNeuralNetwork* Child = NewObject<UNeuralNetwork>(this, UNeuralNetwork::StaticClass());
        TArray<int32> LayerConfig = Parent1->LayerSizes; // Assume both parents share the same configuration.
        Child->Initialize(LayerConfig);

        // Perform uniform crossover: for each weight, randomly select the gene from Parent1 or Parent2.
        for (int32 layer = 0; layer < Child->Weights.Num(); layer++)
        {
            for (int32 neuron = 0; neuron < Child->Weights[layer].Num(); neuron++)
            {
                for (int32 weightIdx = 0; weightIdx < Child->Weights[layer][neuron].Num(); weightIdx++)
                {
                    float RandomValue = FMath::FRand();
                    if (RandomValue < CrossoverProbability)
                    {
                        Child->Weights[layer][neuron][weightIdx] = Parent1->Weights[layer][neuron][weightIdx];
                    }
                    else
                    {
                        Child->Weights[layer][neuron][weightIdx] = Parent2->Weights[layer][neuron][weightIdx];
                    }
                }
            }
        }

        // 3. Dynamic mutation adaptation:
        // Calculate the difference between the best fitness and the average fitness.
        float BestFitness = CurrentGeneration[0]->Fitness;
        float FitnessDiff = BestFitness - OutGenerationFitnessMean;
        // If the difference is small, increase the mutation rate to encourage diversity.
        float DynamicFactor = 1.0f;
        if (FitnessDiff < TargetFitnessDifference)
        {
            DynamicFactor = 1.0f + (TargetFitnessDifference - FitnessDiff) / TargetFitnessDifference; // Factor between 1 and 2.
        }
        float FinalMutationRate = BaseMutationRate * DynamicFactor;

        // Apply mutation to the offspring.
        Child->Mutate(FinalMutationRate);
        NextGeneration.Add(Child);
    }
}
