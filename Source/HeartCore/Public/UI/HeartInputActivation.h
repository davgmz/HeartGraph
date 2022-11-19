﻿// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "HeartInputActivation.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct HEARTCORE_API FHeartInputActivation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Heart|InputActivation")
	double ActivationValue = 0.0;
};
