#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/PostProcessVolume.h"
#include "HarpoonController.h"
#include "Components/TextBlock.h"
#include "Sound.h"

#include "HarpoonGunController.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SHARK_BAIT_API UHarpoonGunController : public UActorComponent
{
    GENERATED_BODY()

public:
    UHarpoonGunController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


    void HandleAim(bool bIsAiming);

    void setCam(UCameraComponent* FirstPersonCameraIn)
    {
        FirstPersonCamera = FirstPersonCameraIn;
        initFov = FirstPersonCamera->FieldOfView;
    }
    void Shoot(FVector forward);
    void Reload();
    void EndReload();
    void ReloadMidpoint();
    void PickupAmmo() {
        ammoCount = maxAmmo;
        Reload();
        updateText();
    };
    int GetAmmoCount() { return ammoCount; }
    void setText(UTextBlock* HarpoonCountIn) {
        HarpoonCount = HarpoonCountIn;
        updateText();
    }
    void updateText();

private:
    UPROPERTY(EditAnywhere, Category = "HarpoonGun")
    FName HarpoonGunComponentName = "HarpoonGun"; 

    UPROPERTY(EditAnywhere, Category = "Harpoon")
    FName HarpoonComponentName = "Harpoon"; 

    APostProcessVolume* PostProcessVolume;

    UStaticMeshComponent* HarpoonGun; 
    UStaticMeshComponent* Harpoon; 

    FTransform DefaultTransform;
    FTransform AimedTransform;

    bool bIsAiming;

    UCameraComponent* FirstPersonCamera;

    bool withoutHarpoon;

    bool bIsReloading = false; 

    FTransform ReloadTransform =  FTransform(FRotator(16, -90, 41), FVector(30.0f, 17.0f, -14.0f), FVector(1, 1, 1));; 

    float ReloadDuration = 1.0f; 
    float ReloadTimeElapsed = 0.0f; 
    enum class EReloadState
    {
        None,
        MovingToReload,
        AtReloadPosition,
        MovingBackToDefault
    };

    EReloadState ReloadState = EReloadState::None;
    float ReloadHoldTime = 0.5f;

    float initSensorWidth;
    float targetSensorWidth = 10.0f;

    float initFov;
    float targetFov = 50.0f;

    UPROPERTY(EditAnywhere, Category = "harpoon")
    AHarpoonController* HarpoonController;
    int ammoCount = 0;
    int maxAmmo = 3;

    UPROPERTY()
    UTextBlock* HarpoonCount;

    UPROPERTY(EditAnywhere, Instanced, Category = "Sound Effects")
    USoundPlayer* shoot;

    UPROPERTY(EditAnywhere, Instanced, Category = "Sound Effects")
    USoundPlayer* start;

    UPROPERTY(EditAnywhere, Instanced, Category = "Sound Effects")
    USoundPlayer* end;

};
