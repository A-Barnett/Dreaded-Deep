#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "Sound.generated.h"

UCLASS(EditInlineNew, BlueprintType)
class SHARK_BAIT_API USoundPlayer : public UObject {
    GENERATED_BODY()

public:

    USoundPlayer() {
        audio = nullptr;
    }

    void RegisterSoundWithActor(AActor* actor);
    void Play() { audio->Play(); }
    void Stop() { audio->Stop(); }

    UPROPERTY(EditAnywhere, Category = "Sound")
    USoundBase* sound;

    UPROPERTY(EditAnywhere, Category = "Sound")
    float volume;

    UPROPERTY(Transient)
    UAudioComponent* audio;
};