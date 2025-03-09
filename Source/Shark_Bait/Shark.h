#pragma once

#include "CoreMinimal.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundSubmix.h"
#include "SubmixEffects/AudioMixerSubmixEffectReverb.h"
#include "Components/ActorComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h" 
#include "Animation/AnimSequence.h"
#include "TimerManager.h"
#include "Sound.h"

#include "Shark.generated.h"


// Sound Setup - By Thomas Johnson
USTRUCT(BlueprintType)
struct SHARK_BAIT_API FRange {
	GENERATED_BODY()

public:

	// Range constructor
	FRange(float min = 0.0f, float max = 1.0f) : m_min(min), m_max(max) {}

	// Returns true if the value is within the range
	inline bool InRange(float value) { return value >= m_min && value < m_max; }

	UPROPERTY(EditAnywhere)
	float m_min;

	UPROPERTY(EditAnywhere)
	float m_max;
};


// Stores information about a sound layer
UCLASS(EditInlineNew, BlueprintType)
class SHARK_BAIT_API USoundLayer : public UObject {
	GENERATED_BODY()

public:

	//Initialises Audio Component to nullptr
	USoundLayer() {
		m_audio = nullptr;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	USoundBase* m_sound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FRange m_range;

	UAudioComponent* m_audio;
};


// Determines the distances at which the sounds are played
struct SHARK_BAIT_API LayerFRanges {
	inline static const FRange LAYER1 = FRange(0, 3200);
	inline static const FRange LAYER2 = FRange(0, 2400);
	inline static const FRange LAYER3 = FRange(0, 1600);
	inline static const FRange LAYER4 = FRange(0, 800);
};
////////////////////////////////////////////////////




UCLASS()
class SHARK_BAIT_API AShark : public AActor
{
	GENERATED_BODY()

public:
	AShark();
	void SharkHit();
	bool isAttacking() { return attacking; }
	void startGame();
	void Death();
	void Initialise();
	void UpdateAmbience();

protected:
	virtual void BeginPlay() override;
	bool init = false;

	UPROPERTY(EditAnywhere, Instanced, Category = "Sound Layers")
	TArray<USoundLayer*> soundLayers;

	UPROPERTY(Transient)
	USoundSubmix* submix;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimSequence* hitAnimation;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimSequence* BeginAnimation;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimSequence* FastAnimation;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimSequence* AttackAnimation;

	USphereComponent* SharkCollider;
	USkeletalMeshComponent* SharkSkeleton;
	FTimerHandle TimerHandle;
	FTimerHandle TimerHandle2;
	FTransform startTransform = FTransform();

	void ReturnToSwimAnimation();

public:
	virtual void Tick(float DeltaTime) override;
private:
	AActor* Player;
	UCapsuleComponent* PlayerCollide;
	bool canSeePlayer;

	bool stalking = true;
	bool attacking = false;
	float distanceToPlayer;
	float stalkTimer;
	float minStalkTime = 12.0f;
	float attackTimer;
	float minAttackTime = 7.5f;

	bool attackAnim = false;

	UPROPERTY(EditAnywhere, Instanced, Category = "Sound Effects")
	USoundPlayer* hurt;

	UPROPERTY(EditAnywhere, Instanced, Category = "Ambience")
	USoundPlayer* ambience;

	UPROPERTY(EditAnywhere, Category = "Ambience")
	float distance;
};