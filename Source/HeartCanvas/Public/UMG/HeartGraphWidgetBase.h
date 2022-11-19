﻿// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "UI/HeartWidgetInputBindingContainer.h"
#include "UI/HeartWidgetInputLinkerRedirector.h"
#include "HeartGraphWidgetBase.generated.h"

/**
 *
 */
UCLASS(Abstract)
class HEARTCANVAS_API UHeartGraphWidgetBase : public UUserWidget, public IHeartWidgetInputLinkerRedirector
{
	GENERATED_BODY()

public:
	UHeartGraphWidgetBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	HEART_WIDGET_INPUT_LINKER_HEADER()

	/** UWidget */
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif
	/** UWidget */

	virtual UHeartWidgetInputLinker* ResolveLinker_Implementation() const override PURE_VIRTUAL(UHeartGraphWidgetBase::ResolveLinker, return nullptr; )

	void GetWidgetActions();
};
