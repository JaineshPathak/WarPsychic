// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputAction.h"
#include "PsychicInputConfigData.generated.h"

/**
 * 
 */
UCLASS()
class ROUGE_LIKE_GAME_API UPsychicInputConfigData : public UDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UInputAction* InputMove;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UInputAction* InputLook;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UInputAction* InputJump;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UInputAction* InputPrimary;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UInputAction* InputDash;
};
