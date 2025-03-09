#include "HarpoonGunController.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "HarpoonController.h"
#include "Engine/World.h"

/***********************************************

HarpoonGunController.cpp - Alex Barnett
Controls the Harpoon Launcher

Handles aiming, shooting, reloading and ammo.

***********************************************/


UHarpoonGunController::UHarpoonGunController()
{
    PrimaryComponentTick.bCanEverTick = true;
    bIsAiming = false; 
}

// Called when the game starts
void UHarpoonGunController::BeginPlay()
{
    Super::BeginPlay();

    if (!PostProcessVolume)
    {
        PostProcessVolume = Cast<APostProcessVolume>(UGameplayStatics::GetActorOfClass(GetWorld(), APostProcessVolume::StaticClass())
        );

        if (PostProcessVolume)
        {
            initSensorWidth = PostProcessVolume->Settings.DepthOfFieldSensorWidth;
        }
    }

    //Find gun and harpoon meshes
    if (!HarpoonGun || !Harpoon) {

        if (AActor* Owner = GetOwner())
        {
            TArray<UStaticMeshComponent*> StaticMeshComponents;
            Owner->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
            for (UStaticMeshComponent* MeshComponent : StaticMeshComponents)
            {
                if (MeshComponent && MeshComponent->GetName() == HarpoonGunComponentName.ToString())
                {
                    HarpoonGun = MeshComponent;
                    DefaultTransform = FTransform(FRotator(0, -81,-5), FVector(30.0f, 17.0f, -2.0f), FVector(1, 1, 1));
                    AimedTransform = FTransform(FRotator(0, -90, 0), FVector(29.0f, 2.55f, -0.5f), FVector(1, 1, 1)); 
                }
                else if (MeshComponent && MeshComponent->GetName() == HarpoonComponentName.ToString()) {
                    Harpoon = MeshComponent;
                }
            }
        }
    }

    if (Harpoon) {
        withoutHarpoon = true;
        Harpoon->SetRelativeLocation(FVector(100000, 0, 0));
    }
    AActor* owner = GetOwner();
    if (owner) {
        shoot->RegisterSoundWithActor(GetOwner());
        start->RegisterSoundWithActor(GetOwner());
        end->RegisterSoundWithActor(GetOwner());
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("NO PLAYER SOUND"));
    }

   
}


void UHarpoonGunController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (ReloadState != EReloadState::None && HarpoonGun)
    {
        ReloadTimeElapsed += DeltaTime;

        if (ReloadState == EReloadState::MovingToReload)
        {
            // Move to the reload position
            float Alpha = FMath::Clamp(ReloadTimeElapsed / ReloadDuration, 0.0f, 1.0f);
            FTransform NewTransform;
            NewTransform.Blend(DefaultTransform, ReloadTransform, Alpha);
            HarpoonGun->SetRelativeTransform(NewTransform);

            if (Alpha >= 1.0f)
            {
                ReloadState = EReloadState::AtReloadPosition;
                ReloadTimeElapsed = 0.0f; 
                ReloadMidpoint();
            }
        }
        else if (ReloadState == EReloadState::AtReloadPosition)
        {
            // Hold at half reload position
            if (ReloadTimeElapsed >= ReloadHoldTime)
            {
                ReloadState = EReloadState::MovingBackToDefault;
                ReloadTimeElapsed = 0.0f;

            }
        }
        else if (ReloadState == EReloadState::MovingBackToDefault)
        {
            // Move back to the default position
            float Alpha = FMath::Clamp(ReloadTimeElapsed / ReloadDuration, 0.0f, 1.0f);
            FTransform NewTransform;
            NewTransform.Blend(ReloadTransform, DefaultTransform, Alpha);
            HarpoonGun->SetRelativeTransform(NewTransform);

            if (Alpha >= 1.0f)
            {
                EndReload();
            }
        }
    }
    else
    {
        // Aiming interpolation
        if (HarpoonGun)
        {
            const FTransform& TargetTransform = bIsAiming ? AimedTransform : DefaultTransform;
            FTransform CurrentTransform = HarpoonGun->GetRelativeTransform();
            FTransform NewTransform = FTransform::Identity;

            NewTransform.Blend(CurrentTransform, TargetTransform, DeltaTime * 5.0f);
            HarpoonGun->SetRelativeTransform(NewTransform);
        }
 
    }
    // Changes depth of field when aiming
    if (PostProcessVolume)
    {
        float CurrentSensorWidth = PostProcessVolume->Settings.DepthOfFieldSensorWidth;
        float TargetSensorWidth = bIsAiming ? targetSensorWidth : initSensorWidth;
        float NewSensorWidth = FMath::Lerp(CurrentSensorWidth, TargetSensorWidth, DeltaTime * 5.0f);
        PostProcessVolume->Settings.bOverride_DepthOfFieldSensorWidth = true;
        PostProcessVolume->Settings.DepthOfFieldSensorWidth = NewSensorWidth;
        PostProcessVolume->MarkComponentsRenderStateDirty();
    }
    if (FirstPersonCamera) {
        float CurrentFov = FirstPersonCamera->FieldOfView;
        float TargetFov = bIsAiming ? targetFov : initFov;
        float NewFov = FMath::Lerp(CurrentFov, TargetFov, DeltaTime * 5.0f);
        FirstPersonCamera->FieldOfView = NewFov;
    }
}


void UHarpoonGunController::HandleAim(bool bNewAimingState)
{
    bIsAiming = bNewAimingState; 
}

void UHarpoonGunController::Shoot(FVector forward)
{
    // Can shoot
    if (Harpoon && !withoutHarpoon)
    {
        shoot->Play();
        Harpoon->SetEnableGravity(false);
        Harpoon->SetSimulatePhysics(true);
        float ImpulseStrength = 500.0f;
        FVector Impulse = forward * ImpulseStrength;
        Harpoon->AddImpulse(Impulse);
        Harpoon->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);

        // Create a new instance of HarpoonController and attach it to the harpoon
        if (!HarpoonController)
        {
            HarpoonController = GetWorld()->SpawnActor<AHarpoonController>(AHarpoonController::StaticClass());
            if (HarpoonController)
            {
                HarpoonController->AttachToComponent(Harpoon, FAttachmentTransformRules::KeepRelativeTransform);
                HarpoonController->setHarpoon(Harpoon);
                Harpoon->OnComponentBeginOverlap.AddDynamic(HarpoonController, &AHarpoonController::OnOverlapBegin);
            }
            HarpoonController = nullptr;
        }
        withoutHarpoon = true;
        ammoCount--;
        updateText();
    }
}

void UHarpoonGunController::Reload()
{
    if (ammoCount <= 0) {
        return;
    }
    // Prevent reloading if already reloading
    if (!withoutHarpoon || ReloadState != EReloadState::None) 
    {
        return;
    }
    if (!HarpoonGun)
    {
        return;
    }

    ReloadState = EReloadState::MovingToReload;
    ReloadTimeElapsed = 0.0f;
    bIsAiming = false;
}

void UHarpoonGunController::ReloadMidpoint() {
    if (Harpoon && GetOwner())
    {
        AActor* Owner = GetOwner();
        UStaticMeshComponent* NewHarpoon = NewObject<UStaticMeshComponent>(Owner, Harpoon->GetClass());

        if (NewHarpoon)
        {
            start->Play();
            NewHarpoon->AttachToComponent(HarpoonGun, FAttachmentTransformRules::KeepRelativeTransform);
            NewHarpoon->SetStaticMesh(Harpoon->GetStaticMesh());
            NewHarpoon->SetMaterial(0, Harpoon->GetMaterial(0));
            NewHarpoon->SetRelativeLocation(FVector::ZeroVector);
            NewHarpoon->SetRelativeRotation(FRotator::ZeroRotator);
            NewHarpoon->SetSimulatePhysics(false);
            NewHarpoon->SetEnableGravity(false);
            NewHarpoon->RegisterComponent();

            Harpoon = NewHarpoon;
            withoutHarpoon = false;
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to reload Harpoon!"));
        }
    }
}

void UHarpoonGunController::EndReload()
{
    end->Play();
    ReloadState = EReloadState::None; // Reset reload state
    ReloadTimeElapsed = 0.0f;
    
}

void UHarpoonGunController::updateText()
{
    if (HarpoonCount)
    {
        FString AmmoText = FString::FromInt(ammoCount) + "/" + FString::FromInt(maxAmmo);
        HarpoonCount->SetText(FText::FromString(AmmoText));
    }
}