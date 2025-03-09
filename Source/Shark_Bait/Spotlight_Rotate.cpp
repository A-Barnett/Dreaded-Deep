#include "Spotlight_Rotate.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/Actor.h"

/************************************************

SpotlightRotate.cpp - Alex Barnett
Rotates Spotlights

Rotates the two spotlights attached to the submarine back and forth

***********************************************/

USpotlight_Rotate::USpotlight_Rotate()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USpotlight_Rotate::BeginPlay()
{
	Super::BeginPlay();

	// Initialize
	Spotlight = GetOwner()->FindComponentByClass<USpotLightComponent>();
	if (!Spotlight)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spotlight component not found on actor!"));
	}
	if (Spotlight)
	{
		FRotator InitialRotation = Spotlight->GetComponentRotation();
		StartAngle = InitialRotation.Yaw;
	}
	if (mirror) {
		bGoingLeft = true;
	}
}


void USpotlight_Rotate::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Spotlight)
	{
		return;
	}


	Timer += DeltaTime;
	float Alpha = FMath::Clamp(Timer / TransitionTime, 0.0f, 1.0f);

	// Calculate new angle
	float TargetAngle = bGoingLeft ? StartAngle - AngleChange : StartAngle + AngleChange;
	float CurrentAngle = FMath::Lerp(bGoingLeft ? StartAngle + AngleChange : StartAngle - AngleChange, TargetAngle, Alpha);

	FRotator NewRotation = Spotlight->GetComponentRotation();
	NewRotation.Yaw = CurrentAngle;
	Spotlight->SetWorldRotation(NewRotation);

	if (Alpha >= 1.0f)
	{
		bGoingLeft = !bGoingLeft; // Reverse direction
		Timer = 0.0f;             
	}
}
