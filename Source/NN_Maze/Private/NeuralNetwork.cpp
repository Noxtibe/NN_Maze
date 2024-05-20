#include "NeuralNetwork.h"
#include <cmath>

void UNeuralNetwork::Initialize(const TArray<int32>& Layers)
{
    LayerSizes = Layers;
    Neurons.SetNum(LayerSizes.Num());
    for (int32 i = 0; i < LayerSizes.Num(); i++)
    {
        Neurons[i].SetNum(LayerSizes[i]);
    }
    Weights.SetNum(LayerSizes.Num() - 1);
    for (int32 i = 0; i < LayerSizes.Num() - 1; i++)
    {
        Weights[i].SetNum(LayerSizes[i + 1]);
        for (int32 j = 0; j < LayerSizes[i + 1]; j++)
        {
            Weights[i][j].SetNum(LayerSizes[i]);
            for (int32 k = 0; k < LayerSizes[i]; k++)
            {
                Weights[i][j][k] = FMath::FRandRange(-1.f, 1.f);
            }
        }
    }

    if (LayerSizes.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("LayerSizes is empty or invalid"));
    }
}

void UNeuralNetwork::CopyWeights(const UNeuralNetwork* SourceNetwork)
{
    LayerSizes = SourceNetwork->LayerSizes;
    Neurons = SourceNetwork->Neurons;
    Weights = SourceNetwork->Weights;
}

TArray<float> UNeuralNetwork::FeedForward(const TArray<float>& Inputs)
{
    if (Inputs.Num() != GetInputSize())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid input size for neural network. Expected: %d, Got: %d"), GetInputSize(), Inputs.Num());
        return TArray<float>();
    }

    for (int32 i = 0; i < Inputs.Num(); i++)
    {
        Neurons[0][i] = Inputs[i];
    }

    for (int32 i = 1; i < LayerSizes.Num(); i++)
    {
        for (int32 j = 0; j < LayerSizes[i]; j++)
        {
            float Value = 0.f;
            for (int32 k = 0; k < LayerSizes[i - 1]; k++)
            {
                Value += Weights[i - 1][j][k] * Neurons[i - 1][k];
            }
            Neurons[i][j] = tanh(Value);
        }
    }

    return Neurons.Last();
}

void UNeuralNetwork::Mutate(float Condition)
{
    for (int32 i = 0; i < Weights.Num(); i++)
    {
        for (int32 j = 0; j < Weights[i].Num(); j++)
        {
            for (int32 k = 0; k < Weights[i][j].Num(); k++)
            {
                if (FMath::FRandRange(0.f, 100.f) <= Condition)
                {
                    Weights[i][j][k] = FMath::FRandRange(-1.f, 1.f);
                }
            }
        }
    }
}

int32 UNeuralNetwork::GetInputSize() const
{
    return LayerSizes.Num() > 0 ? LayerSizes[0] : 0;
}
