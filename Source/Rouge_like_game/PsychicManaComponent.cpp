// Fill out your copyright notice in the Description page of Project Settings.


#include "PsychicManaComponent.h"
#include "PsychicCharacter.h"
#include "PsychicDevUtils.h"
#include "GameFramework/Actor.h"


// Sets default values for this component's properties
UPsychicManaComponent::UPsychicManaComponent() : 
	ManaBase(1000.0f),
	ManaCurrent(0.0f),
	ManaCurrentTarget(0.0f),
	ManaConsumption(100.0f),
	ManaRechargeTime(2.5f),
	ManaRechargeRate(75.0f),

	bJustFired(false),
	bIsManaFull(true),
	ManaRechargeTimeCurrent(0.0f)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UPsychicManaComponent::BeginPlay()
{
	Super::BeginPlay();	

	ManaCurrent = ManaBase;
	ManaCurrentTarget = ManaCurrent;

	//This will trigger the fade out animation in Widget at start, since Mana will always be full 
	OnManaFullEvent.Broadcast();
}

// Called every frame
void UPsychicManaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (PsychicCharacter && PsychicCharacter->GetWorldTimerManager().IsTimerActive(FiredTimerHandle))
	{
		const float elaspedTime = PsychicCharacter->GetWorldTimerManager().GetTimerElapsed(FiredTimerHandle);
		const float totalTime = PsychicCharacter->GetWorldTimerManager().GetTimerRate(FiredTimerHandle);

		ManaCurrent = FMath::Lerp(ManaCurrent, ManaCurrentTarget, elaspedTime / totalTime);
	}

	if (bJustFired)
	{
		ManaRechargeTimeCurrent += DeltaTime;
		if (ManaRechargeTimeCurrent >= ManaRechargeTime)
		{
			bJustFired = false;
			ManaRechargeTimeCurrent = 0.0f;
		}
	}
	else
	{
		ManaCurrent += DeltaTime * ManaRechargeRate;
		ManaCurrent = FMath::Clamp(ManaCurrent, 0.0f, ManaBase);		

		if (!bIsManaFull && ManaCurrent >= ManaBase)
		{
			bIsManaFull = true;
			//DebugLog(-1, 2.0, FColor::Red, "OnManaFull: Broadcast Event - Sent");
			OnManaFullEvent.Broadcast();
		}
	}
}

void UPsychicManaComponent::SetPsychicCharacter(APsychicCharacter* Character)
{
	PsychicCharacter = Character;
	PsychicCharacter->OnItemShoot.AddDynamic(this, &UPsychicManaComponent::OnItemShoot);	
}

void UPsychicManaComponent::OnItemShoot(APsychicCharacter* Character)
{
	//DebugLog(-1, 1.0f, FColor::Cyan, "Item SHOOT! Started Timer");

	if (PsychicCharacter == nullptr)
		PsychicCharacter = Character;

	ManaCurrentTarget = FMath::Clamp(ManaCurrent - ManaConsumption, 0.0f, ManaBase);
	PsychicCharacter->GetWorldTimerManager().SetTimer(FiredTimerHandle, 0.25f, false);
	
	bJustFired = true;
	bIsManaFull = !(ManaCurrentTarget != ManaBase);

	ManaRechargeTimeCurrent = 0.0f;

	OnManaConsumedEvent.Broadcast();
}