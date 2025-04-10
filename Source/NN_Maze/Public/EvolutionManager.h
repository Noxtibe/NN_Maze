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
     * @param CurrentGeneration The array of neural networks to evaluate (fitness values are assumed to be updated).
     * @param NextGeneration    Output array that will be filled with the new generation.
     * @param OutGenerationFitnessMean  Returns the average fitness computed for the generation.
     * @param PopulationSize    The expected size of the population.
     */
    UFUNCTION(BlueprintCallable, Category = "Evolution")
    void ProcessGeneration(TArray<UNeuralNetwork*>& CurrentGeneration, TArray<UNeuralNetwork*>& NextGeneration, float& OutGenerationFitnessMean, int32 PopulationSize);

    // Number of groups into which the population will be divided.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evolution")
    int32 NumGroups;

    // Mutation rates for each group; array size should be equal to NumGroups.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evolution")
    TArray<float> MutationRates;
};
