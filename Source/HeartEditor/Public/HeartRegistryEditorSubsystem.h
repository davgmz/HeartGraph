﻿// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "EditorSubsystem.h"
#include "GraphRegistry/HeartGraphNodeRegistry.h"
#include "HeartRegistryEditorSubsystem.generated.h"

class UHeartEdGraphNode;

/**
 *
 */
UCLASS()
class HEARTEDITOR_API UHeartRegistryEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	/**
	 * Gets the list of classes that should show up in the Common Classes section of the Class Picker Dialog when
	 * creating a new Heart Graph Asset
	 */
	static TArray<UClass*> GetFactoryCommonClasses();

	/**
	 * Creates a slate widget for a EdGraphNode, by looking up a style registered with the Module.
	 */
	TSharedPtr<SGraphNode> MakeVisualWidget(FName Style, UHeartEdGraphNode* Node) const;

protected:
	void BindToRuntimeSubsystem();
	void SubscribeToAssetChanges();

	void FetchAssetRegistryAssets();

	void OnFilesLoaded();
	void OnAssetAdded(const FAssetData& AssetData);
	void OnAssetRemoved(const FAssetData& AssetData);
	void OnHotReload(EReloadCompleteReason ReloadCompleteReason);
	void OnBlueprintPreCompile(UBlueprint* Blueprint);
	void OnBlueprintCompiled();

	void PreRegistryAdded(UHeartGraphNodeRegistry* HeartGraphNodeRegistry);
	void PostRegistryRemoved(UHeartGraphNodeRegistry* HeartGraphNodeRegistry);
	void OnAnyRegistryChanged(UHeartGraphNodeRegistry* HeartGraphNodeRegistry);

	UBlueprint* GetNodeBlueprint(const FAssetData& AssetData) const;

public:
	DECLARE_MULTICAST_DELEGATE(FHeartRegistryEditorPaletteRefresh);
	FHeartRegistryEditorPaletteRefresh OnRefreshPalettes;

private:
	int32 WaitingForBlueprintToCompile;
};
