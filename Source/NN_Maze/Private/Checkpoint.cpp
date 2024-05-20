// Checkpoint.cpp

#include "Checkpoint.h"

ACheckpoint::ACheckpoint()
{
    PrimaryActorTick.bCanEverTick = true;
    RewardMultiplier = 1.0f;
}

void ACheckpoint::BeginPlay()
{
    Super::BeginPlay();
}
