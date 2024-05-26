#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Checkpoint.h"
#include "MazeAgent.generated.h"

UCLASS()
class NN_MAZE_API AMazeAgent : public ACharacter
{
    GENERATED_BODY()

public:

    AMazeAgent();

protected:

    virtual void BeginPlay() override;

public:

    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    void RaycastVision();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
        float RotationSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
        float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vision")
        float MaxViewDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
        float FitnessTimeDecreaseRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning")
        float FitnessCheckpointIncreaseRate;

    UPROPERTY(BlueprintReadWrite, Category = "Learning")
        float Fitness;

    UPROPERTY(BlueprintReadWrite, Category = "Learning")
        bool IsActive;

    UPROPERTY(BlueprintReadWrite, Category = "Learning")
        TArray<float> Outputs;

    UPROPERTY(BlueprintReadWrite, Category = "Input")
        float DistForward;

    UPROPERTY(BlueprintReadWrite, Category = "Input")
        float DistLeft;

    UPROPERTY(BlueprintReadWrite, Category = "Input")
        float DistDiagLeft;

    UPROPERTY(BlueprintReadWrite, Category = "Input")
        float DistRight;

    UPROPERTY(BlueprintReadWrite, Category = "Input")
        float DistDiagRight;

public :

    FVector MoveDir;
    FVector LastPosition;
    float DistanceTraveled;

public:

    UFUNCTION()
        void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    UFUNCTION()
        void OnBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);
};
