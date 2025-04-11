#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NeuralNetwork.h"
#include "EvolutionManager.generated.h"

/**
 * Helper class that encapsulates the evolution algorithm.
 * It processes a generation of neural networks and produces a mutated next generation.
 */
UCLASS(Blueprintable)
class NN_MAZE_API UEvolutionManager : public UObject
{
    GENERATED_BODY()

public:
    UEvolutionManager();

    /**
     * Process the evolution generation.
     *
     * @param CurrentGeneration The array of neural networks with updated fitness values.
     * @param NextGeneration    Output array that will be filled with the new generation.
     * @param OutGenerationFitnessMean  Returns the average fitness computed for the generation.
     * @param PopulationSize    The expected size of the population.
     */
    UFUNCTION(BlueprintCallable, Category = "Evolution")
    void ProcessGeneration(TArray<UNeuralNetwork*>& CurrentGeneration,
        TArray<UNeuralNetwork*>& NextGeneration,
        float& OutGenerationFitnessMean,
        int32 PopulationSize);

    // --- New evolutionary parameters ---

    // The fraction of the population that is kept unchanged (elitism).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evolution")
    float ElitismRate;

    // The base mutation rate applied to offspring.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evolution")
    float BaseMutationRate;

    // The probability to choose a gene from parent1 during crossover (uniform crossover).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evolution")
    float CrossoverProbability;

    // A target fitness difference between the best and average fitness used for dynamic mutation.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evolution")
    float TargetFitnessDifference;
};
