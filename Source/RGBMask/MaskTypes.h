// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "MaskTypes.generated.h"

UENUM(BlueprintType)
enum class EMaskType : uint8
{
    Red   UMETA(DisplayName = "Red"),
    Green UMETA(DisplayName = "Green"),
    Blue  UMETA(DisplayName = "Blue"),
    None  UMETA(DisplayName = "None")
};
