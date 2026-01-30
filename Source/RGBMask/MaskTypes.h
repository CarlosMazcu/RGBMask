// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "MaskTypes.generated.h"

UENUM(BlueprintType)
enum class EMaskType : uint8
{
    Red   UMETA(DisplayName = "Roja"),
    Green UMETA(DisplayName = "Verde"),
    Blue  UMETA(DisplayName = "Azul"),
};
