#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NeuralNetwork.generated.h"

UCLASS(Blueprintable)
class NN_MAZE_API UNeuralNetwork : public UObject
{
    GENERATED_BODY()

public:

    void Initialize(const TArray<int32>& Layers);
    void CopyWeights(const UNeuralNetwork* SourceNetwork);
    TArray<float> FeedForward(const TArray<float>& Inputs) const;
    void Mutate(float Condition);

    UFUNCTION(BlueprintCallable)
        int32 GetInputSize() const;

    UPROPERTY(BlueprintReadWrite)
        float Fitness;

public :

    TArray<int32> LayerSizes;
    TArray<TArray<float>> Neurons;
    TArray<TArray<TArray<float>>> Weights;
};
