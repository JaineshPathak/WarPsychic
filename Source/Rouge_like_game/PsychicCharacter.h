// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "PsychicCharacter.generated.h"

class UCameraComponent;
class USkeletalMeshComponent;
class USpringArmComponent;
class UInputMappingContext;
class UPsychicInputConfigData;
class UPhysicsConstraintComponent;
class UPsychicManaComponent;
class UParticleSystem;
class USoundCue;
class APsychicItem;

UENUM(BlueprintType)
enum class ETeleState : uint8
{
	ETS_Idle UMETA(DisplayName = "Idle"),
	ETS_Scanning UMETA(DisplayName = "Scanning"),
	ETS_Holding UMETA(DisplayName = "Holding"),
	ETS_CanFire UMETA(DisplayName = "Can Fire"),
	ETS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	EMS_Normal UMETA(DisplayName = "Normal"),
	EMS_Dashing UMETA(DisplayName = "Dashing"),
	EMS_MAX UMETA(DisplayName = "DefaultMAX")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemShootSignature, APsychicCharacter*, PsychicCharacter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDashSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDashReadySignature);

UCLASS()
class ROUGE_LIKE_GAME_API APsychicCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APsychicCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Psychic - Input")
	UInputMappingContext* InputMapping;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Psychic - Input")
	UPsychicInputConfigData* InputActions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Psychic - Components", meta = (AllowPrivateAccess = "true"))
	UPsychicManaComponent* ManaComponent;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void PrimaryPressed(const FInputActionValue& Value);
	void PrimaryHold(const FInputActionValue& Value);
	void PrimaryRelease(const FInputActionValue& Value);
	void Dash(const FInputActionValue& Value);

	FVector GetGrabLocation();
	FVector GetSurfaceNormal();

	bool TraceScanForItems();
	void ScanForItems();
	void GrabItem();
	void ReleaseShootItem(bool bShoot = false);

	void ItemGrabInterpStart();
	void ItemGrabInterpFinish();

	void SetMovementState(EMovementState NewState);
	void SetMovementProperties();

public:
	UPROPERTY(BlueprintAssignable, Category = "Psychic - Events")
	FOnItemShootSignature OnItemShoot;

	UPROPERTY(BlueprintAssignable, Category = "Psychic - Events")
	FOnDashSignature OnDash;

	UPROPERTY(BlueprintAssignable, Category = "Psychic - Events")
	FOnDashReadySignature OnDashReady;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Psychic - Camera", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FPHandsMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Psychic - Camera", meta = (AllowPrivateAccess = "true"))
	USceneComponent* FPHandsRoot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Psychic - Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* FPOffsetRoot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Psychic - Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* FPCamRoot;	

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Psychic - Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CameraComponent;

	//===========================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Psychic - Combat", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* FPGrabHeldSlot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Psychic - Combat", meta = (AllowPrivateAccess = "true"))
	UPhysicsConstraintComponent* FPGrabConstraint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Psychic - Combat", meta = (AllowPrivateAccess = "true"))
	ETeleState TeleState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Psychic - Combat", meta = (AllowPrivateAccess = "true"))
	float MaxTeleScanDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Psychic - Combat", meta = (AllowPrivateAccess = "true"))
	float ItemGrabForceAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Psychic - Combat", meta = (AllowPrivateAccess = "true"))
	float ItemReleaseForceAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Psychic - Combat", meta = (AllowPrivateAccess = "true"))
	float ItemMinGrabDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Psychic - Combat", meta = (AllowPrivateAccess = "true"))
	float ItemInterpTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Psychic - Combat", meta = (AllowPrivateAccess = "true"))
	float ItemInterpSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Psychic - Combat", meta = (AllowPrivateAccess = "true"))
	APsychicItem* CurrentItemGrabbed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Psychic - Combat", meta = (AllowPrivateAccess = "true"))
	APsychicItem* CurrentItemLastFrame;

	//===========================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Psychic - Movement", meta = (AllowPrivateAccess = "true"))
	EMovementState MovementState;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Psychic - Movement", meta = (AllowPrivateAccess = "true"))
	//float DashForceAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Psychic - Movement", meta = (AllowPrivateAccess = "true"))
	float DashRayCheckDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Psychic - Movement", meta = (AllowPrivateAccess = "true"))
	float DashResetTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Psychic - Movement", meta = (AllowPrivateAccess = "true"))
	float DashingDistanceThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Psychic - Movement", meta = (AllowPrivateAccess = "true"))
	float DashingInterpTime;

	//===========================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Psychic - FX", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* GrabEffectFXTemplate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Psychic - FX", meta = (AllowPrivateAccess = "true"))
	UParticleSystemComponent* GrabEffectFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Psychic - FX", meta = (AllowPrivateAccess = "true"))
	USoundCue* GrabItemSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Psychic - FX", meta = (AllowPrivateAccess = "true"))
	USoundCue* GrabItemHoldSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Psychic - FX", meta = (AllowPrivateAccess = "true"))
	UAudioComponent* GrabItemHoldSoundComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Psychic - FX", meta = (AllowPrivateAccess = "true"))
	USoundCue* GrabItemShootSound;

	FHitResult TraceScanResult;

	bool bItemIsInterping;
	FTimerHandle ItemGrabInterpTimerHandler;

	bool bCanDash;
	FVector DashMoveToLocation;
	FVector DashVelocity;
	FTimerHandle DashInterpTimerHandler;
	FTimerHandle DashResetTimerHandler;

public:
	FORCEINLINE UCameraComponent* GetCameraComponent() const { return CameraComponent; }
	FORCEINLINE UPsychicManaComponent* GetManaComponent() const { return ManaComponent; }
	FORCEINLINE USceneComponent* GetHandsRoot() const { return FPHandsRoot; }
	
	FORCEINLINE ETeleState GetTeleState() const { return TeleState; }
	FORCEINLINE EMovementState GetMovementState() const { return MovementState; }
};
