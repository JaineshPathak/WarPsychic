// Fill out your copyright notice in the Description page of Project Settings.


#include "PsychicAnimInstance.h"
#include "PsychicCharacter.h"
#include "PsychicDevUtils.h"

UPsychicAnimInstance::UPsychicAnimInstance() :
	bIsIdle(false),
	bIsGrabbing(false),
	bCanFiring(false)
{
}

void UPsychicAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (PsychicCharacter == nullptr)
		PsychicCharacter = Cast<APsychicCharacter>(TryGetPawnOwner());

	if (PsychicCharacter)
	{
		bIsIdle = PsychicCharacter->GetTeleState() == ETeleState::ETS_Idle;
		bIsGrabbing = IsGrabbingItem();
		bCanFiring = PsychicCharacter->GetTeleState() == ETeleState::ETS_CanFire;
	}
}

void UPsychicAnimInstance::NativeInitializeAnimation()
{
	PsychicCharacter = Cast<APsychicCharacter>(TryGetPawnOwner());
}

bool UPsychicAnimInstance::IsGrabbingItem()
{
	if (PsychicCharacter == nullptr)
		return false;

	return  PsychicCharacter->GetTeleState() == ETeleState::ETS_Scanning ||
			PsychicCharacter->GetTeleState() == ETeleState::ETS_Holding;
}
