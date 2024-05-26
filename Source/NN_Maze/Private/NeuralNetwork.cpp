#include "NeuralNetwork.h"
#include <cmath>

void UNeuralNetwork::Initialize(const TArray<int32>& Layers)
{
    if (Layers.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Initialize called with empty Layers array"));
        return;
    }

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
}

void UNeuralNetwork::CopyWeights(const UNeuralNetwork* SourceNetwork)
{
    if (!SourceNetwork || SourceNetwork->LayerSizes.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("SourceNetwork is null or has empty LayerSizes in CopyWeights"));
        return;
    }

    if (SourceNetwork->LayerSizes != LayerSizes)
    {
        UE_LOG(LogTemp, Error, TEXT("SourceNetwork LayerSizes do not match in CopyWeights"));
        return;
    }

    LayerSizes = SourceNetwork->LayerSizes;
    Neurons = SourceNetwork->Neurons;

    Weights.SetNum(SourceNetwork->Weights.Num());
    for (int32 i = 0; i < SourceNetwork->Weights.Num(); i++)
    {
        Weights[i].SetNum(SourceNetwork->Weights[i].Num());
        for (int32 j = 0; j < SourceNetwork->Weights[i].Num(); j++)
        {
            Weights[i][j].SetNum(SourceNetwork->Weights[i][j].Num());
            for (int32 k = 0; k < SourceNetwork->Weights[i][j].Num(); k++)
            {
                Weights[i][j][k] = SourceNetwork->Weights[i][j][k];
            }
        }
    }
}

TArray<float> UNeuralNetwork::FeedForward(const TArray<float>& Inputs) const
{
    if (Inputs.Num() != GetInputSize())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid input size for neural network. Expected: %d, Got: %d"), GetInputSize(), Inputs.Num());
        return TArray<float>();
    }

    TArray<TArray<float>> CurrentNeurons = Neurons;
    for (int32 i = 0; i < Inputs.Num(); i++)
    {
        CurrentNeurons[0][i] = Inputs[i];
    }

    for (int32 i = 1; i < LayerSizes.Num(); i++)
    {
        for (int32 j = 0; j < LayerSizes[i]; j++)
        {
            float Value = 0.f;
            for (int32 k = 0; k < LayerSizes[i - 1]; k++)
            {
                Value += Weights[i - 1][j][k] * CurrentNeurons[i - 1][k];
            }
            CurrentNeurons[i][j] = tanh(Value);
        }
    }

    return CurrentNeurons.Last();
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
    if (LayerSizes.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("LayerSizes is empty in GetInputSize"));
        return 0;
    }

    return LayerSizes[0];
}
