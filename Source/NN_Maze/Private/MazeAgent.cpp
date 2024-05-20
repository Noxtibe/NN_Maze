#include "MazeAgent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Checkpoint.h"

AMazeAgent::AMazeAgent()
{
    PrimaryActorTick.bCanEverTick = true;

    RotationSpeed = 300.f;
    Speed = 0.5f;
    MinVelocity = 0.0f;
    MaxVelocity = 3.0f;
    AccelerationRate = 0.2f;
    DecelerationRate = 0.8f;
    MaxViewDistance = 30.f;
    FitnessTimeDecreaseRate = 10.f;
    FitnessCheckpointIncreaseRate = 100.f;
    Fitness = 0.f;
    IsActive = true;
    CurrentVelocity = 0.f;
    DistanceTraveled = 0.f;

    // Configurez les collisions pour ignorer les raycasts
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
}

void AMazeAgent::BeginPlay()
{
    Super::BeginPlay();
    LastPosition = GetActorLocation();

    if (GetCharacterMovement())
    {
        UE_LOG(LogTemp, Log, TEXT("CharacterMovement component is valid and configured."));
        UE_LOG(LogTemp, Log, TEXT("Max Walk Speed: %f"), GetCharacterMovement()->MaxWalkSpeed);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CharacterMovement component is not valid."));
    }
}

void AMazeAgent::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsActive) return;

    if (Outputs.Num() > 0)
    {
        CurrentVelocity += (AccelerationRate * DeltaTime) * Outputs[0];
        CurrentVelocity = FMath::Clamp(CurrentVelocity, MinVelocity, MaxVelocity);

        FVector MoveDirection = GetActorForwardVector() * CurrentVelocity * DeltaTime;
        FVector NewLocation = GetActorLocation() + MoveDirection;
        SetActorLocation(NewLocation);

        float RotationAngle = Outputs[1] * RotationSpeed * DeltaTime;
        FRotator NewRotation = GetActorRotation();
        NewRotation.Yaw += RotationAngle;
        SetActorRotation(NewRotation);

        // Ajout de messages de debug pour le mouvement
        UE_LOG(LogTemp, Log, TEXT("Agent MoveDir: %s, CurrentVelocity: %.2f, RotationAngle: %.2f"), *MoveDirection.ToString(), CurrentVelocity, RotationAngle);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Outputs array is empty or not set."));
    }

    RaycastVision();

    FVector AgentPosition = GetActorLocation();
    DistanceTraveled += FVector::Dist(AgentPosition, LastPosition);
    LastPosition = AgentPosition;
    if (DistanceTraveled > 0.2f)
    {
        Fitness += DistanceTraveled / 100;
    }
    Fitness -= DeltaTime * FitnessTimeDecreaseRate;

    // Log pour la position et la distance parcourue
    UE_LOG(LogTemp, Log, TEXT("Agent Position: %s, DistanceTraveled: %.2f, Fitness: %.2f"), *AgentPosition.ToString(), DistanceTraveled, Fitness);
}

void AMazeAgent::RaycastVision()
{
    FVector AgentPosition = GetActorLocation();
    FVector ForwardDir = GetActorForwardVector();
    FVector RightDir = GetActorRightVector();
    FVector LeftDir = -RightDir;
    FVector RightDiagDir = (ForwardDir + RightDir).GetSafeNormal();
    FVector LeftDiagDir = (ForwardDir + LeftDir).GetSafeNormal();

    FHitResult Hit;
    FCollisionQueryParams CollisionParams;

    // Forward Ray
    bool bHitForward = GetWorld()->LineTraceSingleByChannel(Hit, AgentPosition, AgentPosition + ForwardDir * MaxViewDistance, ECC_Visibility, CollisionParams);
    DistForward = bHitForward ? Hit.Distance : MaxViewDistance;
    DrawDebugLine(GetWorld(), AgentPosition, AgentPosition + ForwardDir * MaxViewDistance, bHitForward ? FColor::Red : FColor::Green, false, -1, 0, 1);

    // Left Ray
    bool bHitLeft = GetWorld()->LineTraceSingleByChannel(Hit, AgentPosition, AgentPosition + LeftDir * MaxViewDistance, ECC_Visibility, CollisionParams);
    DistLeft = bHitLeft ? Hit.Distance : MaxViewDistance;
    DrawDebugLine(GetWorld(), AgentPosition, AgentPosition + LeftDir * MaxViewDistance, bHitLeft ? FColor::Red : FColor::Green, false, -1, 0, 1);

    // Left Diagonal Ray
    bool bHitLeftDiag = GetWorld()->LineTraceSingleByChannel(Hit, AgentPosition, AgentPosition + LeftDiagDir * MaxViewDistance, ECC_Visibility, CollisionParams);
    DistDiagLeft = bHitLeftDiag ? Hit.Distance : MaxViewDistance;
    DrawDebugLine(GetWorld(), AgentPosition, AgentPosition + LeftDiagDir * MaxViewDistance, bHitLeftDiag ? FColor::Red : FColor::Green, false, -1, 0, 1);

    // Right Ray
    bool bHitRight = GetWorld()->LineTraceSingleByChannel(Hit, AgentPosition, AgentPosition + RightDir * MaxViewDistance, ECC_Visibility, CollisionParams);
    DistRight = bHitRight ? Hit.Distance : MaxViewDistance;
    DrawDebugLine(GetWorld(), AgentPosition, AgentPosition + RightDir * MaxViewDistance, bHitRight ? FColor::Red : FColor::Green, false, -1, 0, 1);

    // Right Diagonal Ray
    bool bHitRightDiag = GetWorld()->LineTraceSingleByChannel(Hit, AgentPosition, AgentPosition + RightDiagDir * MaxViewDistance, ECC_Visibility, CollisionParams);
    DistDiagRight = bHitRightDiag ? Hit.Distance : MaxViewDistance;
    DrawDebugLine(GetWorld(), AgentPosition, AgentPosition + RightDiagDir * MaxViewDistance, bHitRightDiag ? FColor::Red : FColor::Green, false, -1, 0, 1);
}

void AMazeAgent::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AMazeAgent::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
    if (Other->ActorHasTag("Wall"))
    {
        LastPosition = GetActorLocation();
        Fitness -= FitnessCheckpointIncreaseRate;
        IsActive = false;
    }
    else if (Other->ActorHasTag("Checkpoint") && IsActive)
    {
        ACheckpoint* Checkpoint = Cast<ACheckpoint>(Other);
        if (Checkpoint)
        {
            Fitness += FitnessCheckpointIncreaseRate * Checkpoint->RewardMultiplier;
        }
    }
}
