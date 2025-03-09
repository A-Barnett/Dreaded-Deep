#include "MyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h" 
#include "Materials/MaterialInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ProgressBar.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "Blueprint/WidgetTree.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"


/***********************************************

MyCharacter.cpp - Alex Barnett
Controls Most Player Interactions

Includes code for: Movement, Headtorch, Menus, HUD, Artefact Collection.

More code ended up in here than is preferred,
this was due to time constraints making the final stage of development rather rushed.

***********************************************/

AMyCharacter::AMyCharacter()
{

    PrimaryActorTick.bCanEverTick = true;
    MovementComponent = GetCharacterMovement();
    MovementComponent->MaxSwimSpeed = 600.0f;
    MovementComponent->BrakingDecelerationSwimming = 2048.0f;

    // Attach camera
    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(RootComponent);
    FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    FirstPersonCamera->bUsePawnControlRotation = true;
    FirstPersonCamera->FieldOfView = 90;

    HeadTorch = CreateDefaultSubobject<USpotLightComponent>(TEXT("HeadTorch"));
    HeadTorch->SetupAttachment(FirstPersonCamera); 
    HeadTorch->SetRelativeLocation(FVector(37.0f, 0.0f, 15.0f)); 
    HeadTorch->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

    //Spotlight defaults
    HeadTorch->AttenuationRadius = 10000.0f;    
    HeadTorch->InnerConeAngle = 26.5f;         
    HeadTorch->OuterConeAngle = 48.5f; 
    HeadTorch->VolumetricScatteringIntensity = 0.5f;
    HeadTorch->IndirectLightingIntensity = 6.0;
    HeadTorch->InverseExposureBlend = 0.17f;
    HeadTorch->Intensity = 0.0f;

    StartCam = CreateDefaultSubobject<UCameraComponent>(TEXT("StartCam"));
    StartCam->SetupAttachment(RootComponent);

    // Create another HeadTorch for StartCam
    StartCamHeadTorch = CreateDefaultSubobject<USpotLightComponent>(TEXT("StartCamHeadTorch"));
    StartCamHeadTorch->SetupAttachment(StartCam); 
}
    



void AMyCharacter::BeginPlay()
{
    Super::BeginPlay();

    StartCam->SetRelativeTransform(StartCamPosition);
    StartCam->bUsePawnControlRotation = false; 
    StartCam->FieldOfView = 100.0f;
    StartCam->AspectRatio = 2.388889f;
    StartCam->SetConstraintAspectRatio(true);
    StartCam->bConstrainAspectRatio = true;
    StartCam->MarkRenderStateDirty();

    StartCamHeadTorch->SetRelativeLocation(FVector(37.0f, 0.0f, 15.0f)); 
    StartCamHeadTorch->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

    StartCamHeadTorch->AttenuationRadius = 10000.0f;
    StartCamHeadTorch->InnerConeAngle = 26.5f;
    StartCamHeadTorch->OuterConeAngle = 70.0f;
    StartCamHeadTorch->VolumetricScatteringIntensity = 0.2f;
    StartCamHeadTorch->IndirectLightingIntensity = 6.0;
    StartCamHeadTorch->InverseExposureBlend = 0.12f;
    StartCamHeadTorch->IntensityUnits = ELightUnits::Candelas;
    StartCamHeadTorch->Intensity = 104.574371f;

  
}

void AMyCharacter::Initialise() {

    FName Tag = "Dropoff";

    // Finds the Artefact dropoff cubes
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), FoundActors);
    for (AActor* Actor : FoundActors)
    {
        TArray<UActorComponent*> Components = Actor->GetComponentsByTag(UStaticMeshComponent::StaticClass(), Tag);
        for (UActorComponent* Component : Components)
        {
            UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(Component);
            if (MeshComp)
            {
                cubes.Add(MeshComp);
            }
        }
    }


    if (FirstPersonCamera)
    {
        FirstPersonCamera->SetActive(false);
    }
    if (StartCam)
    {
        StartCam->SetActive(true);
        StartCam->SetConstraintAspectRatio(true);
        StartCam->bConstrainAspectRatio = true;
        StartCam->MarkRenderStateDirty();
    }
    if (!HarpoonGunController) {

        HarpoonGunController = FindComponentByClass<UHarpoonGunController>();
        if (!HarpoonGunController)
        {
            UE_LOG(LogTemp, Error, TEXT("HarpoonGunController not found on owner!"));
        }
        else {
            HarpoonGunController->setCam(FirstPersonCamera);
        }

    }
    TArray<AActor*> FoundSharks;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShark::StaticClass(), FoundSharks);
    if (FoundSharks.Num() > 0)
    {
        shark = Cast<AShark>(FoundSharks[0]);
    }

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController)
    {
        PlayerController->bShowMouseCursor = true;
        PlayerController->SetInputMode(FInputModeUIOnly());
    }

    // Create the widgets for UI, this is massively repeated code but I was in a real rush.
    // Just know I hate it too, but all code is from the jam only
    UUserWidget* HUDWidget = CreateWidget<UUserWidget>(PlayerController, CanvasWidgetClass);
    if (HUDWidget)
    {
        HUDWidget->AddToViewport();
        MyHUDWidget = HUDWidget;
        UWidget* progress = MyHUDWidget->GetWidgetFromName("ProgressBarOxygen");
        if (progress)
        {
            ProgressBar = Cast<UProgressBar>(progress);
            if (ProgressBar)
            {
                ProgressBar->SetPercent(1.0f);
                float CurrentPercent = ProgressBar->GetPercent();
            }
        }
        UWidget* text = MyHUDWidget->GetWidgetFromName("Harpoons_Count");
        if (text)
        {
            HarpoonCount = Cast<UTextBlock>(text);
            HarpoonGunController->setText(HarpoonCount);
        }
        MyHUDWidget->SetVisibility(ESlateVisibility::Hidden);

    }

    UUserWidget* HUDWidget2 = CreateWidget<UUserWidget>(PlayerController, StartCanvas);
    if (HUDWidget2)
    {
        HUDWidget2->AddToViewport();
        MyStartWidget = HUDWidget2;
        MyStartWidget->SetVisibility(ESlateVisibility::Visible);
        UWidget* button = MyStartWidget->GetWidgetFromName("BeginButton");
        if (button)
        {
            ButtonWidget = Cast<UButton>(button);
            if (ButtonWidget)
            {
                ButtonWidget->OnClicked.AddDynamic(this, &AMyCharacter::BeginWorld);
            }
        }
    }
    UUserWidget* HUDWidget3 = CreateWidget<UUserWidget>(PlayerController, EndCanvas);
    if (HUDWidget3)
    {
        HUDWidget3->AddToViewport();
        MyEndWidget = HUDWidget3;
        MyEndWidget->SetVisibility(ESlateVisibility::Hidden);
        UWidget* button2 = MyEndWidget->GetWidgetFromName("RestartButton");
        if (button2)
        {
            RestartButtonWidget = Cast<UButton>(button2);
            if (RestartButtonWidget)
            {
                RestartButtonWidget->OnClicked.AddDynamic(this, &AMyCharacter::EndWorld);
            }
        }
        UWidget* button6 = MyEndWidget->GetWidgetFromName("ExitButton");
        if (button6)
        {
            ExitButtonWidget = Cast<UButton>(button6);
            if (ExitButtonWidget)
            {
                ExitButtonWidget->OnClicked.AddDynamic(this, &AMyCharacter::ExitGame);
            }
        }
    }
    UUserWidget* HUDWidget4 = CreateWidget<UUserWidget>(PlayerController, WinCanvas);
    if (HUDWidget4)
    {
        HUDWidget4->AddToViewport();
        MyWinWidget = HUDWidget4;
        MyWinWidget->SetVisibility(ESlateVisibility::Hidden);
        UWidget* button3 = MyWinWidget->GetWidgetFromName("RestartButton");
        if (button3)
        {
            WinRestartButtonWidget = Cast<UButton>(button3);
            if (WinRestartButtonWidget)
            {
                WinRestartButtonWidget->OnClicked.AddDynamic(this, &AMyCharacter::EndWorld);
            }
        }
        UWidget* button6 = MyWinWidget->GetWidgetFromName("ExitButton");
        if (button6)
        {
            ExitButtonWidget = Cast<UButton>(button6);
            if (ExitButtonWidget)
            {
                ExitButtonWidget->OnClicked.AddDynamic(this, &AMyCharacter::ExitGame);
            }
        }
    }

    UUserWidget* HUDWidget5 = CreateWidget<UUserWidget>(PlayerController, PauseCanvas);
    if (HUDWidget5)
    {
        HUDWidget5->AddToViewport();
        MyPauseWidget = HUDWidget5;
        MyPauseWidget->SetVisibility(ESlateVisibility::Hidden);
        UWidget* button4 = MyPauseWidget->GetWidgetFromName("RestartButton");
        if (button4)
        {
            PauseRestartButton = Cast<UButton>(button4);
            if (PauseRestartButton)
            {
                PauseRestartButton->OnClicked.AddDynamic(this, &AMyCharacter::EndWorld);
            }
        }
        UWidget* button5 = MyPauseWidget->GetWidgetFromName("ResumeButton");
        if (button5)
        {
            ResumeButton = Cast<UButton>(button5);
            if (ResumeButton)
            {
                ResumeButton->OnClicked.AddDynamic(this, &AMyCharacter::Resume); //
            }
        }
        UWidget* button6 = MyPauseWidget->GetWidgetFromName("ExitButton");
        if (button6)
        {
            ExitButtonWidget = Cast<UButton>(button6);
            if (ExitButtonWidget)
            {
                ExitButtonWidget->OnClicked.AddDynamic(this, &AMyCharacter::ExitGame);
            }
        }
    }

    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AMyCharacter::OnOverlapBegin);
        GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &AMyCharacter::OnOverlapEnd);
    }

    pickup->RegisterSoundWithActor(this);
    drop->RegisterSoundWithActor(this);
    death->RegisterSoundWithActor(this);
}

void AMyCharacter::BeginWorld()
{
    if (StartCam)
    {
        StartCam->SetActive(false);
    }
    if (FirstPersonCamera)
    {
        FirstPersonCamera->SetActive(true);
    }

    // Enable inputs
    bGameBegun = true;

    MyHUDWidget->SetVisibility(ESlateVisibility::Visible);
    MyStartWidget->SetVisibility(ESlateVisibility::Hidden);
    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (PlayerController)
    {
        EnableInput(PlayerController);

        UInputComponent* PlayerInputComponent = InputComponent; 
        if (PlayerInputComponent)
        {
            PlayerInputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward);
            PlayerInputComponent->BindAxis("MoveRight", this, &AMyCharacter::MoveRight);
            PlayerInputComponent->BindAxis("Height", this, &AMyCharacter::MoveUp);
            PlayerInputComponent->BindAxis("LookUp", this, &AMyCharacter::LookUp);
            PlayerInputComponent->BindAxis("Turn", this, &AMyCharacter::Turn);
            PlayerInputComponent->BindAction("Light", IE_Pressed, this, &AMyCharacter::Light);
            PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AMyCharacter::StartAiming);
            PlayerInputComponent->BindAction("Aim", IE_Released, this, &AMyCharacter::StopAiming);
            PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &AMyCharacter::Shooting);
            PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AMyCharacter::Reloading);
            PlayerInputComponent->BindAction("PickUp", IE_Pressed, this, &AMyCharacter::PickUp);
            PlayerInputComponent->BindAction("Menu", IE_Pressed, this, &AMyCharacter::Resume);
          
            PlayerController->bShowMouseCursor = false;
            PlayerController->SetInputMode(FInputModeGameOnly());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("InputComponent is null. Unable to bind inputs."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerController is null. Unable to enable input."));
    }
    if (shark) {
        shark->startGame();
    }

     if (StartCamHeadTorch) {
          StartCamHeadTorch->Intensity = 0;
          StartCamHeadTorch->MarkRenderStateDirty();
      }

}

void AMyCharacter::PlayerDead(bool win) {
    if (dead) {
        return;
    }
    dead = true;
    death->Play();
    StartCam->SetActive(true);
    FirstPersonCamera->SetActive(false);
    MyHUDWidget->SetVisibility(ESlateVisibility::Hidden);
    MyStartWidget->SetVisibility(ESlateVisibility::Hidden);
    if (win) {
        MyWinWidget->SetVisibility(ESlateVisibility::Visible);
    }
    else {
        MyEndWidget->SetVisibility(ESlateVisibility::Visible);
    }

    shark->startGame();
    StartCamHeadTorch->IntensityUnits = ELightUnits::Candelas;
    StartCamHeadTorch->Intensity = 104.574371f;
    StartCamHeadTorch->MarkRenderStateDirty();

    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    DisableInput(PlayerController);
    if (PlayerController)
    {
        PlayerController->bShowMouseCursor = true;
        PlayerController->SetInputMode(FInputModeUIOnly());
    }
    shark->Death();
}

void AMyCharacter::ExitGame()
{
    UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, true);
}


void AMyCharacter::EndWorld()
{
    FName CurrentLevelName = *GetWorld()->GetName();
    UGameplayStatics::OpenLevel(this, CurrentLevelName);
}

void AMyCharacter::Resume() {
    if (paused) {
        MyHUDWidget->SetVisibility(ESlateVisibility::Visible);
        MyPauseWidget->SetVisibility(ESlateVisibility::Hidden);
        UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
        APlayerController* PlayerController = Cast<APlayerController>(GetController());
        EnableInput(PlayerController);
        if (PlayerController)
        {
            PlayerController->bShowMouseCursor = false;
            PlayerController->SetInputMode(FInputModeGameOnly());
        }
    }
    else {
        MyHUDWidget->SetVisibility(ESlateVisibility::Hidden);
        MyPauseWidget->SetVisibility(ESlateVisibility::Visible);
        UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.001f);
        APlayerController* PlayerController = Cast<APlayerController>(GetController());
        DisableInput(PlayerController);
        if (PlayerController)
        {
            PlayerController->bShowMouseCursor = true;
            PlayerController->SetInputMode(FInputModeUIOnly());
        }
    }
    paused = !paused;
}


void AMyCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && (OtherActor != this) && OtherComp)
    {
        if (OtherComp->ComponentHasTag(FName("Artefact"))) {
            Artefact = OtherActor;
            ArtefactMesh = Cast<UStaticMeshComponent>(Artefact->GetComponentByClass(UStaticMeshComponent::StaticClass()));
            if (ArtefactMesh)
            {
                ArtefactMatOrig = ArtefactMesh->GetMaterial(0);
                if (ArtefactMatNew)
                {
                    ArtefactMesh->SetMaterial(0, ArtefactMatNew);
                }
            }
        }
        else if(OtherComp->ComponentHasTag(FName("ArtefactDropoff"))) {
            inDropoff = true;
            if (ArtefactMesh)
            {
                if (!ArtefactMatOrig) {
                 ArtefactMatOrig = ArtefactMesh->GetMaterial(0);
                }
                if (ArtefactMatNew)
                {
                    ArtefactMesh->SetMaterial(0, ArtefactMatNew);
                }
            }
        }
        else if (OtherComp->ComponentHasTag(FName("Shark")) && shark->isAttacking()) {
            PlayerDead(false);
        }
        else if (OtherComp->ComponentHasTag(FName("Crouch"))) {
            crouching = true;
        }
        else if (OtherComp->ComponentHasTag(FName("Reload"))) {
            inReloadArea = true;
            TArray<USceneComponent*> ChildComponents;
            OtherComp->GetChildrenComponents(true, ChildComponents);
            for (USceneComponent* ChildComp : ChildComponents)
            {
                UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(ChildComp);
                if (MeshComp)
                {
                    MeshComp->SetMaterial(0, HarpoonHilightMat);
                    MeshComp->MarkRenderStateDirty();
                }
            }
        }
    }
}
void AMyCharacter::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && (OtherActor != this) && OtherComp)
    {
        if ((OtherActor == Artefact) && ArtefactMesh) {
            if (ArtefactMatOrig)
            {
                ArtefactMesh->SetMaterial(0, ArtefactMatOrig);
            }
            Artefact = nullptr;
            ArtefactMesh = nullptr;
            ArtefactMatOrig = nullptr;
        }
        else if (OtherComp->ComponentHasTag(FName("ArtefactDropoff"))) {
            inDropoff = false;
            if (ArtefactMesh && ArtefactMatOrig)
            {
                ArtefactMesh->SetMaterial(0, ArtefactMatOrig);
                
            }
        }
        else if (OtherComp->ComponentHasTag(FName("Crouch"))) {
            crouching = false;
        }
        else if (OtherComp->ComponentHasTag(FName("Reload"))) {
            inReloadArea = false;
            TArray<USceneComponent*> ChildComponents;
            OtherComp->GetChildrenComponents(true, ChildComponents);
            for (USceneComponent* ChildComp : ChildComponents)
            {
                UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(ChildComp);
                if (MeshComp)
                {
                    MeshComp->SetMaterial(0, HarpoonInitMat);
                    MeshComp->MarkRenderStateDirty();
                }
            }
        }
    }
}



void AMyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!init) {
        Initialise();
        init = true;
    }
    // Swimming logic
    if (MovementComponent->IsSwimming())
    {
        TargetCameraHeight = 0.0f;
        GetCapsuleComponent()->SetCapsuleHalfHeight(35.0f);
        GetCapsuleComponent()->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f)); 
        oxygenLevel -= DeltaTime;
        if (oxygenLevel <= 0)
        {
            PlayerDead(false);
        }
    }
    else
    {
        GetCapsuleComponent()->SetCapsuleHalfHeight(60.0f);
        GetCapsuleComponent()->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f)); 
        if (crouching)
        {
            TargetCameraHeight = 50.0f; 
        }
        else
        {
            TargetCameraHeight = 100.0f;
        }

        oxygenLevel = maxOxygen;
    }

    CurrentCameraHeight = FMath::Lerp(CurrentCameraHeight, TargetCameraHeight, DeltaTime * LerpSpeed);
    FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, CurrentCameraHeight));

    if (ProgressBar)
    {
        ProgressBar->SetPercent(oxygenLevel / maxOxygen);
    }
}




void AMyCharacter::MoveForward(float Value)
{
    if (Controller && Value != 0.0f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void AMyCharacter::MoveRight(float Value)
{
    if (Controller && Value != 0.0f)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

void AMyCharacter::MoveUp(float Value)
{
    if (Value != 0.0f)
    {
        AddMovementInput(FVector::UpVector, (Value*0.3f));
    }
}

void AMyCharacter::LookUp(float Value)
{
    AddControllerPitchInput(-Value);
}

void AMyCharacter::Turn(float Value)
{
    AddControllerYawInput(Value);
}

void AMyCharacter::Light()
{
    if (bIsFading)
    {
        // Reverse fade 
        bFadeDirection = !bFadeDirection;
        float CurrentIntensity = HeadTorch->Intensity;
        float TargetIntensity = bFadeDirection ? 60000.0f : 0.0f;
        float StartIntensity = bFadeDirection ? 0.0f : 60000.0f;
        CurrentFadeTime = FadeDuration * FMath::Abs(CurrentIntensity - StartIntensity) / FMath::Abs(TargetIntensity - StartIntensity);
        return;
    }

    // New fade
    bIsFading = true;
    bFadeDirection = !torchOn; 
    CurrentFadeTime = 0.0f;

    GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AMyCharacter::HandleLightFade);
}


void AMyCharacter::HandleLightFade()
{

    CurrentFadeTime += GetWorld()->GetDeltaSeconds();
    float Alpha = FMath::Clamp(CurrentFadeTime / FadeDuration, 0.0f, 1.0f);

    float TargetIntensity = bFadeDirection ? 80000.0f : 0.0f;
    float StartIntensity = bFadeDirection ? 0.0f : 80000.0f;
    float NewIntensity = FMath::Lerp(StartIntensity, TargetIntensity, Alpha);

    HeadTorch->SetIntensity(NewIntensity);
    HeadTorch->MarkRenderStateDirty();

    if (Alpha >= 1.0f)
    {
        bIsFading = false; // Fade complete
        torchOn = bFadeDirection; 
    }
    else
    {
        GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AMyCharacter::HandleLightFade);
    }
}

void AMyCharacter::StartAiming()
{
    if (HarpoonGunController)
    {
        HarpoonGunController->HandleAim(true);
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("NO CONTROLLER 1"));
    }
}

void AMyCharacter::StopAiming()
{
    if (HarpoonGunController)
    {
        HarpoonGunController->HandleAim(false);
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("NO CONTROLLER 2"));
    }
}

void AMyCharacter::Shooting() {
    if (HarpoonGunController)
    {
        HarpoonGunController->Shoot(FirstPersonCamera->GetForwardVector());
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("NO CONTROLLER Shoot"));
    }
}

void AMyCharacter::Reloading() {
    if (HarpoonGunController)
    {
        HarpoonGunController->Reload();
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("NO CONTROLLER Reload"));
    }
}

void AMyCharacter::PickUp() {
    if (Artefact && ArtefactMesh && !HeldArtefact)
    {
        HeldArtefact = Artefact;
        HeldArtefactMesh = ArtefactMesh;
        HeldArtefactMesh->SetSimulatePhysics(false);
        HeldArtefactMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        HeldArtefactMesh->SetCollisionProfileName(TEXT("NoCollision"));
        HeldArtefactMesh->AttachToComponent(
            FirstPersonCamera,
            FAttachmentTransformRules::KeepWorldTransform
        );
        HeldArtefactMesh->SetMaterial(0, ArtefactMatOrig);
        HeldArtefactMesh->SetRelativeLocation(FVector(49.0f, -38.0f, -18.0f));
        PointLight = HeldArtefact->GetComponentByClass<UPointLightComponent>();
        if (PointLight) {
            PointLight->Intensity = 100.0f;
            PointLight->MarkRenderStateDirty();
        }
        else {
            UE_LOG(LogTemp, Error, TEXT("NO LIGHT"));
        }
        pickup->Play();

    }
    else if (HeldArtefact && HeldArtefactMesh && inDropoff)
    {
        HeldArtefactMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
        if (cubes.IsValidIndex(artefactCount))
        {
            HeldArtefactMesh->SetWorldLocation(cubes[artefactCount]->GetComponentLocation());
        }

        TArray<UActorComponent*> Components;
        HeldArtefact->GetComponents(Components);
        for (UActorComponent* Component : Components)
        {
            if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Component))
            {
                PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                PrimitiveComponent->SetSimulatePhysics(false);
                PrimitiveComponent->ComponentTags.Empty();
            }
        }
        if (ArtefactMatOrig)
        {
            HeldArtefactMesh->SetMaterial(0, ArtefactMatOrig);
        }

        Artefact = nullptr;
        ArtefactMesh = nullptr;
        ArtefactMatOrig = nullptr;
        HeldArtefact = nullptr;
        HeldArtefactMesh = nullptr;

        drop->Play();

        artefactCount++;
        if (artefactCount >= 6) {
            PlayerDead(true);
        }
    }
    else if (inReloadArea) {
        if (HarpoonGunController)
        {
            HarpoonGunController->PickupAmmo();
            pickup->Play();
        }
    }
}


void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Only bind inputs if the game has begun
    if (bGameBegun)
    {
        PlayerInputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward);
        PlayerInputComponent->BindAxis("MoveRight", this, &AMyCharacter::MoveRight);
        PlayerInputComponent->BindAxis("Height", this, &AMyCharacter::MoveUp);
        PlayerInputComponent->BindAxis("LookUp", this, &AMyCharacter::LookUp);
        PlayerInputComponent->BindAxis("Turn", this, &AMyCharacter::Turn);
        PlayerInputComponent->BindAction("Light", IE_Pressed, this, &AMyCharacter::Light);
        PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AMyCharacter::StartAiming);
        PlayerInputComponent->BindAction("Aim", IE_Released, this, &AMyCharacter::StopAiming);
        PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &AMyCharacter::Shooting);
        PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AMyCharacter::Reloading);
        PlayerInputComponent->BindAction("PickUp", IE_Pressed, this, &AMyCharacter::PickUp);
        PlayerInputComponent->BindAction("Menu", IE_Pressed, this, &AMyCharacter::Resume);
    }
}
