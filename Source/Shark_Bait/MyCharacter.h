#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/SpotLightComponent.h"
#include "HarpoonGunController.h"
#include "Components/CapsuleComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"   
#include "Materials/MaterialInstance.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"
#include "HarpoonController.h"
#include "Components/AudioComponent.h"
#include "Sound.h"
#include "Shark.h"
#include "MyCharacter.generated.h"


UCLASS()
class SHARK_BAIT_API AMyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AMyCharacter();

protected:
    virtual void BeginPlay() override;
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
    void PlayerDead(bool win);
    void MoveForward(float Value);
    void MoveRight(float Value);
    void MoveUp(float Value);
    void LookUp(float Value);
    void Turn(float Value);
    void Light();
    void HandleLightFade();
    void StartAiming();
    void StopAiming();
    void Shooting();
    void Reloading();
    void PickUp();
    void Initialise();
    UFUNCTION()
    void BeginWorld();
    UFUNCTION()
    void EndWorld();
    UFUNCTION()
    void Resume();
    UFUNCTION()
    void ExitGame();
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;


public:
    bool init = false;
    virtual void Tick(float DeltaTime) override;
    FTransform StartCamPosition = FTransform(FRotator(-22.0f, 142.0f, 0.0f), FVector(1222.0f, -2081.0f, 1195.0f), FVector(1, 1, 1));
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    UCameraComponent* StartCam;


private:
    bool paused;
    UPROPERTY(EditAnywhere, Category = "Camera")
    UCameraComponent* FirstPersonCamera;

    UPROPERTY(EditAnywhere, Category = "Movement")
    FVector InitialForce;
    UCharacterMovementComponent* MovementComponent;

    UPROPERTY(EditAnywhere, Category = "SpotLight")
    USpotLightComponent* HeadTorch;
    UPROPERTY(EditAnywhere, Category = "SpotLight")
    USpotLightComponent* StartCamHeadTorch;

    UPROPERTY(EditAnywhere, Category = "HarpoonGunController")
    UHarpoonGunController* HarpoonGunController;

    UPROPERTY(EditAnywhere, Category = "SharkController")
    AShark* shark;

    UPROPERTY(EditAnywhere, Category = "ArtefactDropOff")
    TArray<UStaticMeshComponent*> cubes;

    int artefactCount = 0;

    float FadeDuration = 2.0f;
    float CurrentFadeTime;
    bool bIsFading; 
    bool torchOn;
    bool bFadeDirection; 

    AActor* Artefact;
    UStaticMeshComponent* ArtefactMesh;

    UMaterialInterface* ArtefactMatOrig;

    UPROPERTY(EditAnywhere, Category = "ArtefactMaterial")
    UMaterialInterface* ArtefactMatNew;
    AActor* HeldArtefact;
    UStaticMeshComponent* HeldArtefactMesh;
    UPointLightComponent* PointLight;
    bool inDropoff = false;

    UPROPERTY(EditAnywhere, Category = "harpoon")
    AHarpoonController* HarpoonController;
    UPROPERTY(EditAnywhere, Category = "harpoon")
    UMaterialInstance* HarpoonHilightMat;
    UPROPERTY(EditAnywhere, Category = "harpoon")
    UMaterialInstance* HarpoonInitMat;
    bool inReloadArea;

    UPROPERTY(EditAnywhere,Category = "Oxygen")
    float maxOxygen = 80.0f;

    UPROPERTY(EditAnywhere, Category = "Oxygen")
    float oxygenLevel = 80.0f;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UUserWidget> CanvasWidgetClass;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UUserWidget> StartCanvas;
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UUserWidget> EndCanvas;
    bool dead = false;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UUserWidget> WinCanvas;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UUserWidget> PauseCanvas;


    UPROPERTY()
    UUserWidget* MyHUDWidget;

    UPROPERTY()
    UUserWidget* MyStartWidget;

    UPROPERTY()
    UUserWidget* MyEndWidget;

    UPROPERTY()
    UUserWidget* MyWinWidget;
    UPROPERTY()
    UUserWidget* MyPauseWidget;
    UPROPERTY()
    UProgressBar* ProgressBar;

    UPROPERTY()
    UTextBlock* HarpoonCount;

    UPROPERTY()
    UButton* ButtonWidget;


    UPROPERTY()
    UButton* RestartButtonWidget;

    UPROPERTY()
    UButton* WinRestartButtonWidget;

    UPROPERTY()
    UButton* PauseRestartButton;

    UPROPERTY()
    UButton* ResumeButton;
    UPROPERTY()
    UButton* ExitButtonWidget;
    UPROPERTY()
    UButton* ExitButtonWidget2;
    UPROPERTY()
    UButton* ExitButtonWidget3;

    float CurrentCameraHeight = 100.0f; 
    float TargetCameraHeight = 100.0f; 
    float LerpSpeed = 7.0f; 
    bool crouching = false;
    bool bGameBegun;

    UPROPERTY(EditAnywhere, Instanced, Category = "Sound Effects")
    USoundPlayer* pickup;

    UPROPERTY(EditAnywhere, Instanced, Category = "Sound Effects")
    USoundPlayer* drop;

    UPROPERTY(EditAnywhere, Instanced, Category = "Sound Effects")
    USoundPlayer* death;

};