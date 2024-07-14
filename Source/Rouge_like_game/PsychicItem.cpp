// Fill out your copyright notice in the Description page of Project Settings.


#include "PsychicItem.h"

// Sets default values
APsychicItem::APsychicItem() :
	bSleepAtBeginPlay(true),
	ItemState(EItemState::EIT_Idle),
	ItemThrownTickRate(0.05f),
	ItemThrownVelResetThreshold(10.0f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//ItemSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ItemRoot"));
	//SetRootComponent(ItemSceneRoot);
	//ItemSceneRoot->SetupAttachment(GetRootComponent());

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	//ItemMesh->SetupAttachment(ItemSceneRoot);
	SetRootComponent(ItemMesh);
}

// Called when the game starts or when spawned
void APsychicItem::BeginPlay()
{
	Super::BeginPlay();
	
	SetItemState(bSleepAtBeginPlay ? EItemState::EIT_IdleStatic : EItemState::EIT_Idle);
	SetOutliningStatus(false);
}

// Called every frame
void APsychicItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APsychicItem::SetItemProperties()
{
	if (ItemMesh == nullptr) return;

	switch (ItemState)
	{
	case EItemState::EIT_IdleStatic:
		ItemMesh->SetLinearDamping(0.01f);
		ItemMesh->SetAngularDamping(0.0f);
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Block);

		break;
	case EItemState::EIT_Idle:
		//ItemMesh->SetMassScale(NAME_None, 3.0f);
		ItemMesh->SetLinearDamping(0.01f);
		ItemMesh->SetAngularDamping(0.0f);
		ItemMesh->SetSimulatePhysics(true);
		ItemMesh->SetEnableGravity(true);
		ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Block);
		
		break;

	case EItemState::EIT_Thrown:
		//ItemMesh->SetMassScale(NAME_None, 3.0f);

		SetOutliningStatus(false);

		ItemMesh->SetLinearDamping(0.01f);
		ItemMesh->SetAngularDamping(0.0f);
		ItemMesh->SetSimulatePhysics(true);
		ItemMesh->SetEnableGravity(true);
		ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Block);

		if (GetWorldTimerManager().IsTimerActive(ThrownTimerHandle))
			GetWorldTimerManager().ClearTimer(ThrownTimerHandle);

		GetWorldTimerManager().SetTimer(ThrownTimerHandle, this, &APsychicItem::CheckThrownVelocity, ItemThrownTickRate, true);
		break;

	case EItemState::EIT_Grabbing:
	case EItemState::EIT_Grabbed:
		//ItemMesh->SetMassScale(NAME_None, 1.0f);
		ItemMesh->SetLinearDamping(4.0f);
		ItemMesh->SetAngularDamping(4.0f);
		ItemMesh->SetSimulatePhysics(true);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore);

		if (GetWorldTimerManager().IsTimerActive(ThrownTimerHandle))
			GetWorldTimerManager().ClearTimer(ThrownTimerHandle);
		break;
	}
}

void APsychicItem::CheckThrownVelocity()
{
	if (ItemState != EItemState::EIT_Thrown)
	{
		GetWorldTimerManager().ClearTimer(ThrownTimerHandle);
		return;
	}

	const float ItemVelMag = GetVelocity().Size();
	if (ItemVelMag <= ItemThrownVelResetThreshold)
	{
		GetWorldTimerManager().ClearTimer(ThrownTimerHandle);
		SetItemState(EItemState::EIT_Idle);
	}
}

void APsychicItem::SetOutliningStatus(const bool& status)
{
	if (ItemMesh == nullptr) return;

	ItemMesh->SetRenderCustomDepth(status);
}
