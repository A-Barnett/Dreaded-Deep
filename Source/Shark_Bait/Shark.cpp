#include "Shark.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundSubmix.h"
#include "SubmixEffects/AudioMixerSubmixEffectReverb.h"
#include "MyCharacter.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Color.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"
#include <iostream>
#include <string>

/***********************************************

Shark.cpp - Alex Barnett, with sound by Thomas Johnson where specified
Controls Shark

Controls shark movement, attacking, harpoon hits, and sound.

Shark movement is based on it's vision using rays, and the distance to player. 
If the shark can see the player and is within set range it starts stalking behavior, circling the player.
After a set time stalking it begins an attack, where it will swim quickly directly at the player.
If hit with harpoon it will stop the sharks attack.

***********************************************/


AShark::AShark()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AShark::BeginPlay()
{
    Super::BeginPlay();
}


void AShark::Initialise() {

    // Initialize and play sound layers - By Thomas Johnson
    for (USoundLayer* layer : soundLayers) {
        layer->m_audio = NewObject<UAudioComponent>(this);
        layer->m_audio->RegisterComponentWithWorld(GetWorld());
        layer->m_audio->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        layer->m_audio->SetSubmixSend(submix, 1.0f);
        layer->m_audio->SetSound(layer->m_sound);
        layer->m_audio->SetVolumeMultiplier(0.01f);
    }

    for (USoundLayer* layer : soundLayers) {

        layer->m_audio->Play();
    }
    ////////////////////////////////////////////////////


    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMyCharacter::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0) {
        Player = FoundActors[0];
        if (Player) {
            PlayerCollide = Cast<UCapsuleComponent>(Player->GetComponentByClass(UCapsuleComponent::StaticClass()));
        }
        else UE_LOG(LogTemp, Error, TEXT("Player mesh not found 1!"));
    }


    SharkCollider = GetComponentByClass<USphereComponent>();
    SharkSkeleton = GetComponentByClass<USkeletalMeshComponent>();
    startTransform = SharkCollider->GetComponentTransform();

    hurt->RegisterSoundWithActor(this);
    ambience->RegisterSoundWithActor(this);
    ambience->Play();
}


void AShark::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    if (!init) {
        Initialise();
        init = true;
    }

    // Sound is played based on distance - By Thomas Johnson
	APlayerController* player = UGameplayStatics::GetPlayerController(this, 0);
	FVector camLocation;
	FRotator rotation;
	player->GetPlayerViewPoint(camLocation, rotation);
	FVector sharkLocation = GetActorLocation();
	float distanceVar = FVector::Distance(camLocation, sharkLocation);
	for (USoundLayer* layer : soundLayers) {
		if (!layer->m_range.InRange(distanceVar)) {
			layer->m_audio->SetVolumeMultiplier(0.01f);
			continue;
		}

		float volume = 1 - ((distanceVar - layer->m_range.m_min) / (layer->m_range.m_max - layer->m_range.m_min));

		if (volume < 1e-4) layer->m_audio->SetVolumeMultiplier(0.01f);
		else layer->m_audio->SetVolumeMultiplier(volume);
	}
    /////////////////////////////////////////////////////


   // Shark Movement Logic
    FVector SharkLocation = GetActorLocation();
    FVector ForwardVector = SharkCollider->GetForwardVector();
    FVector RightVector = SharkCollider->GetRightVector();
    FVector UpVector = SharkCollider->GetUpVector();

    float RaySpacing = 400.0f;
    float RayLength = 2000.0f;
    float BaseTurnSpeed = 0.28f;
    float MaxTurnMultiplier = 6.0f;
    float ClosestHitDistance = RayLength;
    //int ObstacleHitCount = 0;

    FVector SteeringDirection = FVector::ZeroVector;
    bool bAvoidingObstacles = false;
    // Obstacle Avoidance
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            FVector Offset =
                ForwardVector * RayLength +
                RightVector * (col - 1) * RaySpacing +
                UpVector * (row - 1) * RaySpacing;

            FVector Start = SharkLocation;
            FVector End = Start + Offset;

            FHitResult HitResult;
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(this);
            QueryParams.AddIgnoredActor(Player);

            bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);

            //FColor LineColor = bHit ? FColor::Red : FColor::Blue;
           // DrawDebugLine(GetWorld(), Start, End, LineColor, false, -1.0f, 0, 1.0f);

            if (bHit) {
                FVector AvoidanceForce = (Start - HitResult.ImpactPoint).GetSafeNormal();
                float HitDistance = (HitResult.ImpactPoint - Start).Size();
                float Weight = FMath::Clamp(1.0f - (HitDistance / RayLength), 0.0f, 1.0f);
                AvoidanceForce *= Weight;

                SteeringDirection += AvoidanceForce;
                ClosestHitDistance = FMath::Min(ClosestHitDistance, HitDistance);
                bAvoidingObstacles = true;
            }
        }
    }

    //// Player Tracking

        FVector PlayerLocation = PlayerCollide->GetComponentLocation();
        distanceToPlayer = GetDistanceTo(Player);

        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this); 
        bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, SharkLocation, PlayerLocation, ECC_Camera, QueryParams);
        
        if (bHit && HitResult.GetActor() == Player) {
            canSeePlayer = true;
            FVector ToPlayer = (PlayerLocation - SharkLocation).GetSafeNormal();
            float PlayerWeight = 3000000.0f; // Increase this value to prioritize turning toward the player
            //SteeringDirection += (ToPlayer * PlayerWeight)*((9-ObstacleHitCount)/9);
            SteeringDirection += (ToPlayer * PlayerWeight);
            FVector Start = SharkLocation;
            float HitDistance = (HitResult.ImpactPoint - Start).Size();
            ClosestHitDistance = FMath::Min(ClosestHitDistance, HitDistance);
        }
        else {
            canSeePlayer = false;
        }
        //FColor LineColor = (bHit && HitResult.GetActor() == Player) ? FColor::Green : FColor::Red;
        //DrawDebugLine(GetWorld(), SharkLocation, PlayerLocation, LineColor, false, -1.0f, 0, 1.0f);
    if (!SteeringDirection.IsZero()) {
        SteeringDirection.Normalize();
    }

    // Apply Smoothing and Turning
    static FVector SmoothedSteeringDirection = FVector::ZeroVector;
    float SmoothingFactor = attacking?1.0f:0.1f;
    SmoothedSteeringDirection = FMath::Lerp(SmoothedSteeringDirection, SteeringDirection, SmoothingFactor);
    float TurnSpeedMultiplier = 0.0f;
    if (stalking && !bAvoidingObstacles) {
        TurnSpeedMultiplier = 0.97f;
        BaseTurnSpeed = 0.2f;
    }
    else if (attacking && !bAvoidingObstacles) {
        TurnSpeedMultiplier = 0.0f;
    }
    else {
        TurnSpeedMultiplier = FMath::Clamp(ClosestHitDistance / RayLength, 0.1f, 1.0f);
    }
    float MaxTurnSpeed = BaseTurnSpeed + (1.0f - TurnSpeedMultiplier) * (MaxTurnMultiplier - BaseTurnSpeed);

    if (!SmoothedSteeringDirection.IsZero()) {
        SmoothedSteeringDirection.Normalize();
        FVector DesiredForwardVector = (ForwardVector + SmoothedSteeringDirection).GetSafeNormal();

        FRotator CurrentRotation = GetActorRotation();
        FRotator TargetRotation = DesiredForwardVector.Rotation();
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), MaxTurnSpeed);
       
        if (NewRotation.Pitch > 20.0f) {
            NewRotation.Pitch = 20.0f;
        }
        else if (NewRotation.Pitch < -20.0f) {
            NewRotation.Pitch = -20.0f;
        }
        SetActorRotation(NewRotation);
    }

    if (stalking && !attacking) {
        SharkCollider->SetLinearDamping(1.0f);
        SharkCollider->SetAngularDamping(1.0f);
        if (distanceToPlayer < 4300 && canSeePlayer) {
            // In Distance
            stalkTimer += DeltaTime;
            if (stalkTimer > minStalkTime) {
                stalking = false;
                attacking = true;
                stalkTimer = 0.0f;
                SharkSkeleton->PlayAnimation(FastAnimation, true);
            }
        }
        else {
           // Out of Distance
            stalkTimer -= DeltaTime;
            if (stalkTimer < 0.0f) {
                stalkTimer = 0.0f;
            }
        }
    }
    else {
        SharkCollider->SetLinearDamping(9.0f);
        SharkCollider->SetAngularDamping(9.0f);
        // Attacking
        attackTimer += DeltaTime;
        if (attackTimer > minAttackTime) {
            stalking = true;
            attacking = false;
            attackTimer = 0.0f;
            SharkSkeleton->PlayAnimation(BeginAnimation, true);
        }
    }
    if (distanceToPlayer < 800 && !attackAnim && attacking) {

        SharkSkeleton->PlayAnimation(AttackAnimation, false);
        float AttackAnimDuration = AttackAnimation->GetPlayLength();
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AShark::ReturnToSwimAnimation, AttackAnimDuration, false);
        attackAnim = true;
    }
    if(distanceToPlayer < 4300){
        SharkCollider->AddImpulse(ForwardVector* (attacking ? 950 : 60));
    }
    else {
        SharkCollider->AddImpulse(ForwardVector *  (60.0f+ (distanceToPlayer/100.0f)));
    }
}

void AShark::SharkHit() {
    FVector ForwardVector = SharkCollider->GetForwardVector();
    SharkCollider->AddImpulse(ForwardVector * -10000);
    stalking = true;
    attacking = false;
    attackTimer = 0.0f;
    if (SharkSkeleton && hitAnimation)
    {
        // Play the hit animation
        SharkSkeleton->PlayAnimation(hitAnimation, false);
        float HitAnimDuration = hitAnimation->GetPlayLength();
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AShark::ReturnToSwimAnimation, HitAnimDuration, false);
    }
    hurt->Play();
}

void AShark::ReturnToSwimAnimation()
{
    if (SharkSkeleton && BeginAnimation)
    {
        // Play the swimming animation on loop
        SharkSkeleton->PlayAnimation(BeginAnimation, true);
        attackAnim = false;
    }
}

void AShark::startGame() {
    //Reset position on menu "Begin" select
    if (SharkCollider) {
        SharkCollider->SetWorldTransform(startTransform);
    }
}

void AShark::Death() {
    stalking = true;
    attacking = false;
    attackTimer = 0.0f;
    ambience->Stop();
    for (USoundLayer* layer : soundLayers) {
        layer->m_audio->Stop();
    }
}
