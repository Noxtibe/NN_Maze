#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Checkpoint.generated.h"

UCLASS(Blueprintable)
class NN_MAZE_API ACheckpoint : public AActor
{
	GENERATED_BODY()
	
public:

    ACheckpoint();

protected:

    virtual void BeginPlay() override;

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
    float RewardMultiplier;

};
