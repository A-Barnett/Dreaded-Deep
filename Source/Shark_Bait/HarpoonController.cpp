#include "HarpoonController.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Shark.h"

/***********************************************

HarpoonController.cpp - Alex Barnett
Controls the harpoon once it has been shot

***********************************************/


AHarpoonController::AHarpoonController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AHarpoonController::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && (OtherActor != this) && OtherComp)
    {
        if (OtherComp->ComponentHasTag(FName("Shark")))
        {
            AShark* Shark = Cast<AShark>(OtherActor);
            if (Shark)
            {
                Shark->SharkHit();  
                Harpoon->SetSimulatePhysics(false);
                Harpoon->SetEnableGravity(false);
                Harpoon->SetCollisionEnabled(ECollisionEnabled::NoCollision);

                // Attach the harpoon to the shark
                Harpoon->AttachToComponent(OtherComp, FAttachmentTransformRules::KeepWorldTransform);
            }
        }
    }
}
