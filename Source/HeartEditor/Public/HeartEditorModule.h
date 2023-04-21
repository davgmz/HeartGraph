﻿// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "AssetTypeCategories.h"
#include "IAssetTypeActions.h"
#include "Modules/ModuleInterface.h"
#include "PropertyEditorDelegates.h"
#include "Toolkits/IToolkit.h"

class FSlateStyleSet;
struct FGraphPanelPinConnectionFactory;

class FHeartGraphAssetEditor;
class UHeartGraph;

DECLARE_LOG_CATEGORY_EXTERN(LogHeartEditor, Log, All)

class HEARTEDITOR_API FHeartEditorModule : public IModuleInterface
{
public:
    static EAssetTypeCategories::Type HeartAssetCategory_TEMP;

private:
    TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetActions;
    TSet<FName> CustomClassLayouts;

public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    void RegisterAssets();
    void UnregisterAssets();

    void RegisterPropertyCustomizations();
    void RegisterCustomClassLayout(const TSubclassOf<UObject> Class, const FOnGetDetailCustomizationInstance DetailLayout);

public:
    FDelegateHandle ModulesChangedHandle;

private:
    void ModulesChangesCallback(FName ModuleName, EModuleChangeReason ReasonForChange);
    void RegisterAssetIndexers() const;

    /** Property Customizations; Cached so they can be unregistered */
    TMap<FName, FOnGetPropertyTypeCustomizationInstance> PropertyCustomizations;

public:
    static TSharedRef<FHeartGraphAssetEditor> CreateHeartGraphAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UHeartGraph* HeartGraph);
};
