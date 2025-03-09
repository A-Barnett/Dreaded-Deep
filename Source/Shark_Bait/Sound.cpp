#include "Sound.h"

/***********************************************

Sound.cpp - Thomas Johnson
Handles Sound Effects

***********************************************/

void USoundPlayer::RegisterSoundWithActor(AActor* actor) {
    if (audio == nullptr) {
        delete audio;
    }

    audio = NewObject<UAudioComponent>(actor);
    audio->RegisterComponentWithWorld(actor->GetWorld());
    audio->AttachToComponent(actor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
    audio->SetSound(sound);
    audio->SetVolumeMultiplier(volume);
}