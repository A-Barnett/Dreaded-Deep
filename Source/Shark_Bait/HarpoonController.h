#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"

#include "HarpoonController.generated.h"

UCLASS()
class SHARK_BAIT_API AHarpoonController : public AActor
{
	GENERATED_BODY()
	
public:	
	AHarpoonController();
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
	void setHarpoon(UStaticMeshComponent* HarpoonIn) {
		Harpoon = HarpoonIn;
	}
private:
	UStaticMeshComponent* Harpoon;
};
