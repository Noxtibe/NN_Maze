#include "MazeAgent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Checkpoint.h"
#include "Kismet/KismetMathLibrary.h"

AMazeAgent::AMazeAgent()
{
    PrimaryActorTick.bCanEverTick = true;

    RotationSpeed = 300.f;
    Speed = 1.0f;
    MaxViewDistance = 30.f;
    FitnessTimeDecreaseRate = 10.f;
    FitnessCheckpointIncreaseRate = 100.f;
    Fitness = 0.f;
    IsActive = true;
    DistanceTraveled = 0.f;
    NeuralNet = nullptr; // To be assigned by MazeManager during spawn

    // Configure collisions
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
    GetCapsuleComponent()->SetNotifyRigidBodyCollision(true);

    // Bind hit events
    GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AMazeAgent::OnHit);
    OnActorBeginOverlap.AddDynamic(this, &AMazeAgent::OnBeginOverlap);

    // Initialize optimization variables
    LastLogTime = 0.f;
    LastRaycastUpdateTime = 0.f;
    RaycastUpdateInterval = 0.1f;

    // Initialize sensor smoothing parameters
    SensorSmoothingFactor = 0.3f; // Adjust as needed (0 = no update, 1 = full raw value)
    PrevDistForward = MaxViewDistance;
    PrevDistLeft = MaxViewDistance;
    PrevDistDiagLeft = MaxViewDistance;
    PrevDistRight = MaxViewDistance;
    PrevDistDiagRight = MaxViewDistance;
}

void AMazeAgent::BeginPlay()
{
    Super::BeginPlay();
    LastPosition = GetActorLocation();

    if (GetCharacterMovement())
    {
        UE_LOG(LogTemp, Log, TEXT("CharacterMovement component is valid. Max Walk Speed: %f"), GetCharacterMovement()->MaxWalkSpeed);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CharacterMovement component is invalid."));
    }
}

void AMazeAgent::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsActive)
        return;

    // Update sensor data via raycast (with smoothing)
    RaycastVision();

    // Process neural network output if assigned
    if (NeuralNet)
    {
        ProcessNeuralNetwork();
    }

    // Update fitness based on distance traveled and time penalty using dedicated functions.
    FVector CurrentPosition = GetActorLocation();
    float DeltaDistance = FVector::Dist(CurrentPosition, LastPosition);
    DistanceTraveled += DeltaDistance;
    LastPosition = CurrentPosition;

    if (DeltaDistance > 0.2f)
    {
        ApplyDistanceReward(DeltaDistance);
    }

    ApplyTimePenalty(DeltaTime);
}

void AMazeAgent::ProcessNeuralNetwork()
{
    // Normalize sensor inputs
    float NormalizedSpeed = Speed / MaxViewDistance;
    float NormForward = DistForward / MaxViewDistance;
    float NormLeft = DistLeft / MaxViewDistance;
    float NormDiagLeft = DistDiagLeft / MaxViewDistance;
    float NormRight = DistRight / MaxViewDistance;
    float NormDiagRight = DistDiagRight / MaxViewDistance;

    TArray<float> Inputs = { NormalizedSpeed, NormForward, NormLeft, NormDiagLeft, NormRight, NormDiagRight };

    if (Inputs.Num() != NeuralNet->GetInputSize())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid input size for neural network. Expected: %d, Got: %d"),
            NeuralNet->GetInputSize(), Inputs.Num());
        return;
    }

    // Feed inputs through the neural network
    TArray<float> NNOutputs = NeuralNet->FeedForward(Inputs);
    if (NNOutputs.Num() < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("Insufficient neural network outputs."));
        return;
    }

    // Interpret outputs: first value as speed multiplier, second as rotation delta
    float SpeedMultiplier = NNOutputs[0];
    float RotationDelta = NNOutputs[1];

    // Move the agent according to the neural network output
    FVector MoveDelta = GetActorForwardVector() * Speed * SpeedMultiplier * GetWorld()->GetDeltaSeconds();
    SetActorLocation(GetActorLocation() + MoveDelta);

    // Apply rotation
    FRotator NewRotation = GetActorRotation();
    NewRotation.Yaw += RotationDelta * RotationSpeed * GetWorld()->GetDeltaSeconds();
    SetActorRotation(NewRotation);

    //UE_LOG(LogTemp, Log, TEXT("NeuralNet output: SpeedMultiplier=%.2f, RotationDelta=%.2f"), SpeedMultiplier, RotationDelta);
}

void AMazeAgent::RaycastVision()
{
    // Only perform raycasts if the update interval has elapsed.
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastRaycastUpdateTime < RaycastUpdateInterval)
    {
        return;
    }
    LastRaycastUpdateTime = CurrentTime;

    FVector AgentLocation = GetActorLocation();
    FVector ForwardDir = GetActorForwardVector();
    FVector RightDir = GetActorRightVector();
    FVector LeftDir = -RightDir;
    FVector RightDiagDir = (ForwardDir + RightDir).GetSafeNormal();
    FVector LeftDiagDir = (ForwardDir + LeftDir).GetSafeNormal();

    FHitResult Hit;
    FCollisionQueryParams CollisionParams;

    // Define a lambda that performs a line trace and returns the distance.
    auto PerformRayCast = [this, AgentLocation, &CollisionParams, &Hit](const FVector& Direction) -> float
        {
            bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, AgentLocation, AgentLocation + Direction * MaxViewDistance, ECC_Visibility, CollisionParams);
            return bHit ? Hit.Distance : MaxViewDistance;
        };

    // Get raw sensor readings.
    float RawForward = PerformRayCast(ForwardDir);
    float RawLeft = PerformRayCast(LeftDir);
    float RawLeftDiag = PerformRayCast(LeftDiagDir);
    float RawRight = PerformRayCast(RightDir);
    float RawRightDiag = PerformRayCast(RightDiagDir);

    // Apply smoothing to raw sensor readings using Lerp and clamp the values.
    DistForward = FMath::Clamp(FMath::Lerp<float>(PrevDistForward, RawForward, SensorSmoothingFactor), 0.f, MaxViewDistance);
    DistLeft = FMath::Clamp(FMath::Lerp<float>(PrevDistLeft, RawLeft, SensorSmoothingFactor), 0.f, MaxViewDistance);
    DistDiagLeft = FMath::Clamp(FMath::Lerp<float>(PrevDistDiagLeft, RawLeftDiag, SensorSmoothingFactor), 0.f, MaxViewDistance);
    DistRight = FMath::Clamp(FMath::Lerp<float>(PrevDistRight, RawRight, SensorSmoothingFactor), 0.f, MaxViewDistance);
    DistDiagRight = FMath::Clamp(FMath::Lerp<float>(PrevDistDiagRight, RawRightDiag, SensorSmoothingFactor), 0.f, MaxViewDistance);

    // Update previous sensor values for use in the next update cycle.
    PrevDistForward = DistForward;
    PrevDistLeft = DistLeft;
    PrevDistDiagLeft = DistDiagLeft;
    PrevDistRight = DistRight;
    PrevDistDiagRight = DistDiagRight;
}

void AMazeAgent::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// --- Dedicated fitness modification functions ---

void AMazeAgent::ApplyDistanceReward(float DeltaDistance)
{
    // Reward agent for moving a distance greater than a threshold.
    Fitness += DeltaDistance / 100.f;
}

void AMazeAgent::ApplyTimePenalty(float DeltaTime)
{
    // Penalize agent over time
    Fitness -= DeltaTime * FitnessTimeDecreaseRate;
}

void AMazeAgent::ApplyWallPenalty()
{
    // Apply penalty when hitting a wall; you can adjust the penalty value as needed.
    Fitness -= FitnessCheckpointIncreaseRate;
}

void AMazeAgent::ApplyCheckpointReward(float RewardMultiplier)
{
    // Reward the agent for reaching a checkpoint using the multiplier provided by the checkpoint.
    Fitness += FitnessCheckpointIncreaseRate * RewardMultiplier;
}

void AMazeAgent::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (OtherActor && (OtherActor != this) && OtherComp)
    {
        if (OtherActor->ActorHasTag("Wall"))
        {
            ApplyWallPenalty();
            IsActive = false;
        }
    }
}

void AMazeAgent::OnBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
    if (OtherActor && (OtherActor != this))
    {
        if (OtherActor->ActorHasTag("Checkpoint") && IsActive)
        {
            ACheckpoint* Checkpoint = Cast<ACheckpoint>(OtherActor);
            if (Checkpoint)
            {
                ApplyCheckpointReward(Checkpoint->RewardMultiplier);
            }
        }
    }
}