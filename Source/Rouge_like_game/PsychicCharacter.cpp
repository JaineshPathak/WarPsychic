// Fill out your copyright notice in the Description page of Project Settings.


#include "PsychicCharacter.h"
#include "PsychicDevUtils.h"
#include "PsychicItem.h"
#include "PsychicManaComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "PsychicInputConfigData.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AudioComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
APsychicCharacter::APsychicCharacter() :
	TeleState(ETeleState::ETS_Idle),

	MaxTeleScanDistance(1000.0f),
	ItemGrabForceAmount(650000.0f),
	ItemReleaseForceAmount(700000.0f),
	ItemMinGrabDistance(100.0f),

	ItemInterpTime(1.0f),
	ItemInterpSpeed(30.0f),

	//DashForceAmount(15.0f),
	DashRayCheckDistance(300.0f),
	DashResetTime(0.5f),
	DashingDistanceThreshold(15.0f),
	DashingInterpTime(20.0f),
	bCanDash(true),

	bItemIsInterping(false)
{
 	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	FPHandsRoot = CreateDefaultSubobject<USceneComponent>(TEXT("FPHandsRoot"));
	FPHandsRoot->SetupAttachment(GetCapsuleComponent());
	FPHandsRoot->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));

	FPOffsetRoot = CreateDefaultSubobject<USpringArmComponent>(TEXT("FPOffsetRoot"));
	FPOffsetRoot->SetupAttachment(FPHandsRoot);
	FPOffsetRoot->TargetArmLength = 0.0f;
	FPOffsetRoot->bDoCollisionTest = false;
	FPOffsetRoot->bUsePawnControlRotation = true;
	FPOffsetRoot->bInheritPitch = true;
	FPOffsetRoot->bInheritYaw = true;
	FPOffsetRoot->bInheritRoll = false;
	FPOffsetRoot->SetComponentTickEnabled(false);

	FPHandsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPHandsMesh"));
	FPHandsMesh->SetOnlyOwnerSee(true);
	FPHandsMesh->SetupAttachment(FPOffsetRoot);
	FPHandsMesh->bCastDynamicShadow = false;
	FPHandsMesh->CastShadow = false;
	FPHandsMesh->SetRelativeLocation(FVector(5.0f, 0.f, -180.0f));
	FPHandsMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	FPCamRoot = CreateDefaultSubobject<USpringArmComponent>(TEXT("FPCamRoot"));
	FPCamRoot->SetupAttachment(FPHandsRoot);
	FPCamRoot->TargetArmLength = 0.0f;
	FPCamRoot->bDoCollisionTest = false;
	FPCamRoot->bUsePawnControlRotation = true;
	FPCamRoot->bInheritPitch = true;
	FPCamRoot->bInheritYaw = true;
	FPCamRoot->bInheritRoll = false;
	FPCamRoot->SetComponentTickEnabled(false);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FPCam"));
	CameraComponent->SetupAttachment(FPCamRoot);
	CameraComponent->SetRelativeLocation(FVector::ZeroVector);
	CameraComponent->bUsePawnControlRotation = true;

	FPGrabHeldSlot = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FPGrabHeldSlot"));
	FPGrabHeldSlot->SetupAttachment(CameraComponent);
	FPGrabHeldSlot->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
	FPGrabHeldSlot->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FPGrabHeldSlot->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	FPGrabHeldSlot->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FPGrabHeldSlot->SetHiddenInGame(true);

	//Physics Handle
	FPGrabConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("FPGrabHandle"));
	FPGrabConstraint->SetupAttachment(FPGrabHeldSlot);
	
	FConstraintInstance& cInstance = FPGrabConstraint->ConstraintInstance;
	cInstance.SetLinearXMotion(ELinearConstraintMotion::LCM_Free);
	cInstance.SetLinearYMotion(ELinearConstraintMotion::LCM_Free);
	cInstance.SetLinearZMotion(ELinearConstraintMotion::LCM_Free);
	cInstance.SetAngularTwistMotion(EAngularConstraintMotion::ACM_Free);
	cInstance.SetLinearPositionDrive(true, true, true);
	cInstance.SetLinearVelocityDrive(true, true, true);
	cInstance.SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);
	cInstance.SetOrientationDriveTwistAndSwing(true, true);
	cInstance.SetAngularVelocityDriveTwistAndSwing(true, true);

	ManaComponent = CreateDefaultSubobject<UPsychicManaComponent>(TEXT("PsychicManaComponent"));

	GetCharacterMovement()->AirControl = 0.5f;
}

// Called when the game starts or when spawned
void APsychicCharacter::BeginPlay()
{
	Super::BeginPlay();	
	
	//Setup Input Mapping Context
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(InputMapping, 0);
		}
	}

	if(FPGrabHeldSlot) FPGrabHeldSlot->SetWorldLocation(GetGrabLocation());
	if(ManaComponent) ManaComponent->SetPsychicCharacter(this);

	if (GrabEffectFXTemplate)
	{
		GrabEffectFX = UGameplayStatics::SpawnEmitterAttached(GrabEffectFXTemplate,
			FPHandsMesh,
			TEXT("PsyGrabEffectSocket"),
			FVector::ZeroVector, FRotator::ZeroRotator, FVector(0.0005f, 0.0005f, 0.0005f),
			EAttachLocation::KeepRelativeOffset, false, EPSCPoolMethod::None, false);
	}

	if (GrabItemHoldSound)
		GrabItemHoldSoundComp = UGameplayStatics::CreateSound2D(this, GrabItemHoldSound, 1.0f, 1.0f, 0.0f, nullptr, false, false);
}

// Called every frame
void APsychicCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//DrawDebugSphere(GetWorld(), GetGrabLocation(), 5.0f, 8, FColor::Blue, false, -1.0f);
	if(TeleState == ETeleState::ETS_Idle || TeleState == ETeleState::ETS_Scanning && CurrentItemGrabbed == nullptr)
		TraceScanForItems();

	if (bItemIsInterping && CurrentItemGrabbed != nullptr)
	{
		const FVector CurrentLocation = CurrentItemGrabbed->GetActorLocation();
		const FVector EndLocation = GetGrabLocation();

		FVector InterpedLocation = FMath::VInterpTo(CurrentLocation, EndLocation, DeltaTime, ItemInterpSpeed);

		CurrentItemGrabbed->SetActorLocation(InterpedLocation);
	}

	if (MovementState == EMovementState::EMS_Dashing)
	{
		const FVector CurrentPlayerLocation = GetActorLocation();
		
		FVector DashInterpLocation = FMath::VInterpTo(CurrentPlayerLocation, DashMoveToLocation, DeltaTime, DashingInterpTime);
		SetActorLocation(DashInterpLocation);

		/*float Distance = FVector::Distance(GetActorLocation(), DashMoveToLocation);
		if (Distance <= DashingDistanceThreshold)
		{
			GetCharacterMovement()->GravityScale = 1.0f;
			GetCharacterMovement()->AirControl = 0.5f;
			MovementState = EMovementState::EMS_Normal;
		}*/
	}

	/*FPGrabConstraint->ConstraintInstance.SetLinearPositionTarget(GetGrabLocation());
	if (CurrentItemGrabbed != nullptr && TeleState == ETeleState::ETS_CanFire)
	{
		FPGrabHeldSlot->SetWorldLocation(CurrentItemGrabbed->GetActorLocation(), false, nullptr, ETeleportType::TeleportPhysics);
	}*/

	/*if (bMouseHolding)
	{
		if (TeleState == ETeleState::ETS_Scanning && CurrentItemGrabbed == nullptr)
			ScanForItems();
		else if (TeleState == ETeleState::ETS_Holding || TeleState == ETeleState::ETS_CanFire && CurrentItemGrabbed != nullptr)
			GrabItem();
	}*/

	//DebugLogVal(-1, -1.0, FColor::Cyan, "Current State: ", UEnum::GetValueAsString<ETeleState>(TeleState));
	//if (GEngine)
		//GEngine->AddOnScreenDebugMessage(-1, -1.0, FColor::Red, FString::Printf("Current State: %s", TeleState));

	//UE_LOG(LogTemp, Log, TEXT("Current State: %s"), *UEnum::GetValueAsName(TeleState).ToString());
}

// Called to bind functionality to input
void APsychicCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Get the EnhancedInputComponent
	if (UEnhancedInputComponent* PEI = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		PEI->BindAction(InputActions->InputJump, ETriggerEvent::Started, this, &ACharacter::Jump);
		PEI->BindAction(InputActions->InputJump, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		
		PEI->BindAction(InputActions->InputDash, ETriggerEvent::Started, this, &APsychicCharacter::Dash);

		PEI->BindAction(InputActions->InputMove, ETriggerEvent::Triggered, this, &APsychicCharacter::Move);
		PEI->BindAction(InputActions->InputLook, ETriggerEvent::Triggered, this, &APsychicCharacter::Look);

		PEI->BindAction(InputActions->InputPrimary, ETriggerEvent::Started, this, &APsychicCharacter::PrimaryPressed);
		PEI->BindAction(InputActions->InputPrimary, ETriggerEvent::Ongoing, this, &APsychicCharacter::PrimaryHold);
		PEI->BindAction(InputActions->InputPrimary, ETriggerEvent::Completed, this, &APsychicCharacter::PrimaryRelease);
	}
}

void APsychicCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void APsychicCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookValue = Value.Get<FVector2D>();
	
	if (Controller != nullptr)
	{
		AddControllerYawInput(LookValue.X);
		AddControllerPitchInput(LookValue.Y);
	}
}

void APsychicCharacter::PrimaryPressed(const FInputActionValue& Value)
{
	//DebugLog(-1, 2.0f, FColor::Red, "Primary - Pressed");
	if (TeleState != ETeleState::ETS_Idle) return;

	TeleState = ETeleState::ETS_Scanning;
	
	if (GrabEffectFX && ManaComponent->HasEnoughMana()) 
		GrabEffectFX->Activate();
}

void APsychicCharacter::PrimaryHold(const FInputActionValue& Value)
{
	//DebugLog(-1, -1.0f, FColor::Green, "Primary - Hold");
	switch (TeleState)
	{
		case ETeleState::ETS_Scanning: 
			if (ManaComponent && ManaComponent->HasEnoughMana()) { ScanForItems(); }
			break;
		case ETeleState::ETS_Holding:
		case ETeleState::ETS_CanFire:
			GrabItem();
			break;
	}
}

void APsychicCharacter::PrimaryRelease(const FInputActionValue& Value)
{
	//DebugLog(-1, 2.0f, FColor::Blue, "Primary - Released");

	switch (TeleState)
	{
	case ETeleState::ETS_Scanning:
		TeleState = ETeleState::ETS_Idle;

		CurrentItemGrabbed = nullptr;
		CurrentItemLastFrame = nullptr;

		if (GrabItemHoldSoundComp)
			GrabItemHoldSoundComp->FadeOut(1.0f, 0.0f);
		break;
	case ETeleState::ETS_Holding:
		if (CurrentItemGrabbed != nullptr && CurrentItemGrabbed->GetItemState() == EItemState::EIT_Grabbing)
		{
			TeleState = ETeleState::ETS_Idle;
			
			CurrentItemGrabbed->SetItemState(EItemState::EIT_Idle);
			CurrentItemGrabbed = nullptr;
			CurrentItemLastFrame = nullptr;
		}
		break;
	case ETeleState::ETS_CanFire:
		if (CurrentItemGrabbed != nullptr && CurrentItemGrabbed->GetItemState() == EItemState::EIT_Grabbed)
		{
			TeleState = ETeleState::ETS_Idle;

			ReleaseShootItem(ManaComponent->HasEnoughMana());
		}
		break;
	default:
		TeleState = ETeleState::ETS_Idle;
		
		CurrentItemGrabbed = nullptr;
		CurrentItemLastFrame = nullptr;
	}

	if (GrabEffectFX) GrabEffectFX->Deactivate();
}

void APsychicCharacter::Dash(const FInputActionValue& Value)
{
	if (MovementState == EMovementState::EMS_Dashing || !bCanDash) return;

	//FVector DashVelocity = (GetLastMovementInputVector().Size() > 0.0f ? GetLastMovementInputVector() : GetActorForwardVector()) * 7500.0f;
	//LaunchCharacter(DashVelocity, false, false);	

	const FVector Start = GetActorLocation();
	FVector End = GetActorLocation() + ((GetLastMovementInputVector().Size() > 0.0f ? GetLastMovementInputVector() : GetActorForwardVector()) * DashRayCheckDistance) + GetSurfaceNormal();
	
	DashVelocity = GetLastMovementInputVector().Size() > 0.0f ? GetLastMovementInputVector() : GetActorForwardVector();
	DashMoveToLocation = End;

	FHitResult WallHitResult;
	bool HitSuccess = GetWorld()->LineTraceSingleByChannel(WallHitResult, Start, DashMoveToLocation, ECollisionChannel::ECC_Visibility);
	if (HitSuccess)
		DashMoveToLocation = WallHitResult.ImpactPoint + WallHitResult.ImpactNormal * GetCapsuleComponent()->GetScaledCapsuleRadius();

	//DrawDebugSphere(GetWorld(), DashMoveToLocation, 5.0f, 8, FColor::Red, false, 2.0f);

	SetMovementState(EMovementState::EMS_Dashing);
	GetWorldTimerManager().SetTimer(DashInterpTimerHandler, FTimerDelegate::CreateLambda([&]
		{
			SetMovementState(EMovementState::EMS_Normal);
			GetCharacterMovement()->Velocity = DashVelocity * 500.0f;
		}), DashingInterpTime / 100.0f, false);
	
	bCanDash = false;
	GetWorldTimerManager().SetTimer(DashResetTimerHandler, FTimerDelegate::CreateLambda([&]
		{
			OnDashReady.Broadcast();
			bCanDash = true;
		}), DashResetTime, false);

	OnDash.Broadcast();
}

FVector APsychicCharacter::GetGrabLocation()
{
	return CameraComponent->GetComponentLocation() + (CameraComponent->GetForwardVector() * 200.0f - CameraComponent->GetRightVector() * 50.0f + CameraComponent->GetUpVector() * 75.0f);
}

FVector APsychicCharacter::GetSurfaceNormal()
{
	if (!GetCapsuleComponent()) return FVector::ZeroVector;

	FVector ReturnNormal = FVector::ZeroVector;

	const FVector Start = GetActorLocation();
	const FVector End = GetActorLocation() - FVector(0.0f, 0.0f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

	//DrawDebugSphere(GetWorld(), End, 5.0f, 8, FColor::Blue, false, 2.0f);

	FHitResult HitResult;
	bool HitSuccess = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility);
	if (HitSuccess) 
		ReturnNormal = HitResult.ImpactNormal;

	return ReturnNormal;
}

bool APsychicCharacter::TraceScanForItems()
{
	const FVector TraceStart = CameraComponent->GetComponentLocation();
	const FVector TraceEnd = CameraComponent->GetComponentLocation() + (CameraComponent->GetForwardVector() * MaxTeleScanDistance);

	FCollisionQueryParams HitCollisionParams;
	HitCollisionParams.AddIgnoredActor(this);
	
	bool HitSuccess = GetWorld()->LineTraceSingleByChannel(TraceScanResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, HitCollisionParams);
	if (HitSuccess)
	{
		CurrentItemGrabbed = Cast<APsychicItem>(TraceScanResult.GetActor());
		if ( (CurrentItemGrabbed) && (CurrentItemGrabbed->GetItemState() == EItemState::EIT_Idle || CurrentItemGrabbed->GetItemState() == EItemState::EIT_IdleStatic || CurrentItemGrabbed->GetItemState() == EItemState::EIT_Thrown))
			CurrentItemGrabbed->SetOutliningStatus(true);

		if (CurrentItemLastFrame && CurrentItemLastFrame != CurrentItemGrabbed)
			CurrentItemLastFrame->SetOutliningStatus(false);

		CurrentItemLastFrame = CurrentItemGrabbed;
	}
	else
	{
		if (CurrentItemLastFrame)
		{
			CurrentItemLastFrame->SetOutliningStatus(false);

			CurrentItemGrabbed = nullptr;
			CurrentItemLastFrame = nullptr;
		}
	}

	return HitSuccess;
}

void APsychicCharacter::ScanForItems()
{
	//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, HitSuccess ? FColor::Blue : FColor::Red, false, 5.0f, 0, 10.0f);
	if (TraceScanForItems() && CurrentItemGrabbed)
	{
		if (GrabItemSound)
			UGameplayStatics::PlaySound2D(this, GrabItemSound);

		TeleState = ETeleState::ETS_Holding;
		CurrentItemGrabbed->SetItemState(EItemState::EIT_Grabbing);		
	}

	if (GrabItemHoldSoundComp && !GrabItemHoldSoundComp->IsPlaying())
		GrabItemHoldSoundComp->FadeIn(1.0f);

	if (GrabEffectFX)
	{
		if (!GrabEffectFX->IsActive())
			GrabEffectFX->Activate();
	}
}

void APsychicCharacter::GrabItem()
{
	FVector GrabDir = GetGrabLocation() - CurrentItemGrabbed->GetActorLocation();
	GrabDir.Normalize();

	if (CurrentItemGrabbed->GetItemState() == EItemState::EIT_Grabbing)
	{
		CurrentItemGrabbed->GetItemMesh()->AddTorqueInDegrees(FVector(-360.0f, 360.0f, 0.0f) * ItemGrabForceAmount);
		CurrentItemGrabbed->GetItemMesh()->AddForce(GrabDir * ItemGrabForceAmount, NAME_None, false);
		//CurrentItemGrabbed->GetItemMesh()->AddForceAtLocation(GrabDir * ItemGrabForceAmount, CurrentItemGrabbed->GetItemMesh()->GetCenterOfMass(), NAME_None);

		float distance = FVector::Distance(GetGrabLocation(), CurrentItemGrabbed->GetActorLocation());		
		if (distance <= ItemMinGrabDistance)
		{						
			//CurrentItemGrabbed->GetItemMesh()->WakeRigidBody();			
			//FPGrabConstraint->SetConstrainedComponents(FPGrabHeldSlot, NAME_None, CurrentItemGrabbed->GetItemMesh(), NAME_None);
			ItemGrabInterpStart();

			TeleState = ETeleState::ETS_CanFire;
			CurrentItemGrabbed->SetItemState(EItemState::EIT_Grabbed);
		}
	}
}

void APsychicCharacter::ReleaseShootItem(bool bShoot)
{
	bItemIsInterping = false;
	if(GetWorldTimerManager().IsTimerActive(ItemGrabInterpTimerHandler))
		GetWorldTimerManager().ClearTimer(ItemGrabInterpTimerHandler);

	FPGrabConstraint->BreakConstraint();

	CurrentItemGrabbed->SetItemState(EItemState::EIT_Idle);
	if (bShoot)
	{
		const FVector TraceStart = CameraComponent->GetComponentLocation();
		const FVector TraceEnd = CameraComponent->GetComponentLocation() + (CameraComponent->GetForwardVector() * 5000.0f);
		FVector ImpactLocation = TraceEnd;

		FCollisionQueryParams ShootCollisionParams;
		ShootCollisionParams.AddIgnoredActor(this);
		ShootCollisionParams.AddIgnoredActor(CurrentItemGrabbed);
		
		FHitResult ShootResult;
		if (GetWorld()->LineTraceSingleByChannel(ShootResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, ShootCollisionParams))
			ImpactLocation = ShootResult.ImpactPoint;

		FVector ShootVelocity = ImpactLocation - CurrentItemGrabbed->GetActorLocation();
		ShootVelocity.Normalize();

		//DrawDebugSphere(GetWorld(), ImpactLocation, 5.0f, 8, FColor::Magenta, false, 2.0f);
		//DrawDebugLine(GetWorld(), TraceStart, ImpactLocation, FColor::Magenta, true, 2.0f);
		
		CurrentItemGrabbed->GetItemMesh()->AddImpulse(ShootVelocity * ItemReleaseForceAmount);
		CurrentItemGrabbed->SetItemState(EItemState::EIT_Thrown);

		OnItemShoot.Broadcast(this);
	}
	else
		CurrentItemGrabbed->SetItemState(EItemState::EIT_Idle);

	CurrentItemGrabbed = nullptr;
	CurrentItemLastFrame = nullptr;

	if (GrabItemShootSound)
		UGameplayStatics::PlaySound2D(this, GrabItemShootSound);

	if (GrabItemHoldSoundComp)
		GrabItemHoldSoundComp->FadeOut(1.0f, 0.0f);
}

void APsychicCharacter::ItemGrabInterpStart()
{
	bItemIsInterping = true;
	
	if (GetWorldTimerManager().IsTimerActive(ItemGrabInterpTimerHandler))
		GetWorldTimerManager().ClearTimer(ItemGrabInterpTimerHandler);

	GetWorldTimerManager().SetTimer(ItemGrabInterpTimerHandler, this, &APsychicCharacter::ItemGrabInterpFinish, ItemInterpTime);
}

void APsychicCharacter::ItemGrabInterpFinish()
{
	bItemIsInterping = false;

	if(CurrentItemGrabbed)
	{
		CurrentItemGrabbed->GetItemMesh()->WakeRigidBody();
		FPGrabConstraint->SetConstrainedComponents(FPGrabHeldSlot, NAME_None, CurrentItemGrabbed->GetItemMesh(), NAME_None);
	}
}

void APsychicCharacter::SetMovementState(EMovementState NewState)
{
	MovementState = NewState;
	SetMovementProperties();
}

void APsychicCharacter::SetMovementProperties()
{
	switch (MovementState)
	{
	case EMovementState::EMS_Normal:
		GetCharacterMovement()->GravityScale = 1.0f;
		GetCharacterMovement()->AirControl = 0.5f;
		break;
	case EMovementState::EMS_Dashing:
		GetCharacterMovement()->GravityScale = 0.0f;
		GetCharacterMovement()->AirControl = 0.0f;
		break;
	}
}