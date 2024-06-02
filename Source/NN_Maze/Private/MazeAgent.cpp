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
    Speed = 1.0f; // Vitesse de base de l'agent
    MaxViewDistance = 30.f;
    FitnessTimeDecreaseRate = 10.f;
    FitnessCheckpointIncreaseRate = 100.f;
    Fitness = 0.f;
    IsActive = true;
    DistanceTraveled = 0.f;

    // Configurez les collisions pour ignorer les raycasts
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
    GetCapsuleComponent()->SetNotifyRigidBodyCollision(true); // Enable hit events

    // Bind the OnHit function to the component's hit event
    GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AMazeAgent::OnHit);

    // Bind the OnBeginOverlap function to the actor's overlap event
    OnActorBeginOverlap.AddDynamic(this, &AMazeAgent::OnBeginOverlap);
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
        FVector MoveDirection = GetActorForwardVector() * Speed * Outputs[0] * DeltaTime;
        FVector NewLocation = GetActorLocation() + MoveDirection;
        SetActorLocation(NewLocation);

        float RotationAngle = Outputs[1] * RotationSpeed * DeltaTime;
        FRotator NewRotation = GetActorRotation();
        NewRotation.Yaw += RotationAngle;
        SetActorRotation(NewRotation);

        // Ajout de messages de debug pour le mouvement
        UE_LOG(LogTemp, Log, TEXT("Agent MoveDir: %s, Speed: %.2f, RotationAngle: %.2f"), *MoveDirection.ToString(), Speed, RotationAngle);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Outputs array is empty or not set."));
    }

    RaycastVision();

    FVector AgentPosition = GetActorLocation();
    float DistanceDelta = FVector::Dist(AgentPosition, LastPosition);
    DistanceTraveled += DistanceDelta;
    LastPosition = AgentPosition;
    if (DistanceDelta > 0.2f)
    {
        Fitness += DistanceDelta / 100;
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

    auto PerformRaycast = [&](const FVector& Direction, float& Distance)
    {
        bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, AgentPosition, AgentPosition + Direction * MaxViewDistance, ECC_Visibility, CollisionParams);
        Distance = bHit ? Hit.Distance : MaxViewDistance;
    };

    PerformRaycast(ForwardDir, DistForward);
    PerformRaycast(LeftDir, DistLeft);
    PerformRaycast(LeftDiagDir, DistDiagLeft);
    PerformRaycast(RightDir, DistRight);
    PerformRaycast(RightDiagDir, DistDiagRight);
}

void AMazeAgent::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AMazeAgent::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (OtherActor && (OtherActor != this) && OtherComp)
    {
        if (OtherActor->ActorHasTag("Wall"))
        {
            LastPosition = GetActorLocation();
            Fitness -= FitnessCheckpointIncreaseRate;
            IsActive = false;

            // Debug message when hitting a wall
            /*if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Hit a wall!"));
            }*/
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
                Fitness += FitnessCheckpointIncreaseRate * Checkpoint->RewardMultiplier;

                // Debug message when hitting a checkpoint
                /*if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Hit a checkpoint!"));
                }*/
            }
        }
    }
}
