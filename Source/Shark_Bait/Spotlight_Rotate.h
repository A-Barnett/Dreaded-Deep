#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SpotLightComponent.h"
#include "Spotlight_Rotate.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SHARK_BAIT_API USpotlight_Rotate : public UActorComponent
{
	GENERATED_BODY()

public:
	USpotlight_Rotate();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	float StartAngle;

	UPROPERTY(EditAnywhere, Category = "Rotation")
	float AngleChange = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Rotation")
	float TransitionTime = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Rotation")
	bool mirror = false;

	float Timer = 0.0f;
	bool bGoingLeft = false;

	USpotLightComponent* Spotlight;
};
