﻿// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "GraphRegistry/GraphNodeRegistrar.h"
#include "GraphRegistry/HeartGraphNodeRegistry.h"
#include "GraphRegistry/HeartRegistryRuntimeSubsystem.h"
#include "Model/HeartGraphNode.h"
#include "View/HeartVisualizerInterfaces.h"

bool UHeartGraphNodeRegistry::FilterClassForRegistration(const TObjectPtr<UClass>& Class) const
{
	return IsValid(Class) ? !(Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated)) : false;
}

void UHeartGraphNodeRegistry::AddRegistrationList(const FHeartRegistrationClasses& Registration)
{
	for (auto&& GraphNodeList : Registration.GraphNodeLists)
	{
		if (GraphNodeList.Value.NodeClasses.IsEmpty())
		{
			continue;
		}

		auto&& CountedList = GraphClasses.FindOrAdd(GraphNodeList.Key);

		for (auto&& NodeClass : GraphNodeList.Value.NodeClasses)
		{
			if (!FilterClassForRegistration(NodeClass))
			{
				continue;
			}

			CountedList.FindOrAdd(NodeClass)++;
		}
	}

	for (auto&& NodeVisualizerClass : Registration.NodeVisualizerClasses)
	{
		if (!FilterClassForRegistration(NodeVisualizerClass))
		{
			continue;
		}

		if (UClass* SupportedClass =
				IGraphNodeVisualizerInterface::Execute_GetSupportedGraphNodeClass(NodeVisualizerClass->GetDefaultObject()))
		{
			NodeVisualizerMap.FindOrAdd(SupportedClass).FindOrAdd(NodeVisualizerClass)++;
		}
	}

	for (auto&& PinVisualizerClass : Registration.PinVisualizerClasses)
	{
		if (!FilterClassForRegistration(PinVisualizerClass))
		{
			continue;
		}

		FHeartGraphPinTag SupportedTag =
			IGraphPinVisualizerInterface::Execute_GetSupportedGraphPinTag(PinVisualizerClass->GetDefaultObject());
		if (SupportedTag.IsValid())
		{
			PinVisualizerMap.FindOrAdd(SupportedTag).FindOrAdd(PinVisualizerClass)++;
		}
	}
}

void UHeartGraphNodeRegistry::RemoveRegistrationList(const FHeartRegistrationClasses& Registration)
{
	for (auto&& GraphNodeList : Registration.GraphNodeLists)
	{
		auto* CountedList = GraphClasses.Find(GraphNodeList.Key);
		if (CountedList == nullptr) continue;

		for (auto&& NodeClass : GraphNodeList.Value.NodeClasses)
		{
			int32* ClassCount = CountedList->Find(NodeClass);
			if (ClassCount == nullptr) continue;

			(*ClassCount)--;
			if (*ClassCount <= 0)
			{
				CountedList->Remove(NodeClass);
				if (CountedList->IsEmpty())
				{
					GraphClasses.Remove(GraphNodeList.Key);
				}
			}
		}
	}

	for (auto&& NodeVisualizerClass : Registration.NodeVisualizerClasses)
	{
		if (!IsValid(NodeVisualizerClass))
		{
			continue;
		}

		if (UClass* SupportedClass =
			IGraphNodeVisualizerInterface::Execute_GetSupportedGraphNodeClass(NodeVisualizerClass->GetDefaultObject()))
		{
			NodeVisualizerMap.Remove(SupportedClass);
		}
	}

	for (auto&& PinVisualizerClass : Registration.PinVisualizerClasses)
	{
		if (!IsValid(PinVisualizerClass))
		{
			continue;
		}

		FHeartGraphPinTag SupportedTag =
			IGraphPinVisualizerInterface::Execute_GetSupportedGraphPinTag(PinVisualizerClass->GetDefaultObject());
		if (SupportedTag.IsValid())
		{
			PinVisualizerMap.Remove(SupportedTag);
		}
	}
}

void UHeartGraphNodeRegistry::SetRecursivelyDiscoveredClasses(const FHeartRegistrationClasses& Classes)
{
	FHeartRegistrationClasses ClassesToRemove;

	for (auto&& NodeClass : RecursivelyDiscoveredClasses.NodeClasses)
	{
		if (!Classes.NodeClasses.Contains(NodeClass))
		{
			ClassesToRemove.NodeClasses.Add(NodeClass);
		}
	}

	for (auto&& GraphNodeClass : RecursivelyDiscoveredClasses.GraphNodeClasses)
	{
		if (!Classes.GraphNodeClasses.Contains(GraphNodeClass))
		{
			ClassesToRemove.GraphNodeClasses.Add(GraphNodeClass);
		}
	}

	for (auto&& NodeVisualizerClass : RecursivelyDiscoveredClasses.NodeVisualizerClasses)
	{
	    if (!Classes.NodeVisualizerClasses.Contains(NodeVisualizerClass))
	    {
		    ClassesToRemove.NodeVisualizerClasses.Add(NodeVisualizerClass);
	    }
	}

    for (auto&& PinVisualizerClass : RecursivelyDiscoveredClasses.PinVisualizerClasses)
    {
        if (!RecursivelyDiscoveredClasses.PinVisualizerClasses.Contains(PinVisualizerClass))
        {
        	ClassesToRemove.PinVisualizerClasses.Add(PinVisualizerClass);
        }
    }

	RemoveRegistrationList(ClassesToRemove);

	FHeartRegistrationClasses ClassesToAdd;

	for (auto&& NodeClass : Classes.NodeClasses)
	{
		if (!RecursivelyDiscoveredClasses.NodeClasses.Contains(NodeClass))
		{
			ClassesToAdd.NodeClasses.Add(NodeClass);
		}
	}

	for (auto&& GraphNodeClass : Classes.GraphNodeClasses)
	{
		if (!RecursivelyDiscoveredClasses.GraphNodeClasses.Contains(GraphNodeClass))
		{
			ClassesToAdd.GraphNodeClasses.Add(GraphNodeClass);
		}
	}

	for (auto&& NodeVisualizerClass : Classes.NodeVisualizerClasses)
	{
		if (!RecursivelyDiscoveredClasses.NodeVisualizerClasses.Contains(NodeVisualizerClass))
		{
			ClassesToAdd.NodeVisualizerClasses.Add(NodeVisualizerClass);
		}
	}

	for (auto&& PinVisualizerClass : Classes.PinVisualizerClasses)
	{
		if (!Classes.PinVisualizerClasses.Contains(PinVisualizerClass))
		{
			ClassesToAdd.PinVisualizerClasses.Add(PinVisualizerClass);
		}
	}

	AddRegistrationList(ClassesToAdd);
}

FHeartRegistrationClasses UHeartGraphNodeRegistry::GetClassesRegisteredRecursively()
{
	FHeartRegistrationClasses Classes;

	for (auto&& Registrar : ContainedRegistrars)
	{
		if (Registrar->Recursive)
		{
			Classes.NodeClasses.Append(Registrar->Registration.NodeClasses);
			Classes.GraphNodeClasses.Append(Registrar->Registration.GraphNodeClasses);
			Classes.NodeVisualizerClasses.Append(Registrar->Registration.NodeVisualizerClasses);
			Classes.PinVisualizerClasses.Append(Registrar->Registration.PinVisualizerClasses);
		}
	}

	return Classes;
}

void UHeartGraphNodeRegistry::ForEachNodeObjectClass(const TFunctionRef<bool(TSubclassOf<UHeartGraphNode>, UClass*)>& Iter)
{
	for (auto&& GraphClassList : GraphClasses)
	{
		TSubclassOf<UHeartGraphNode> GraphNodeClass = GraphClassList.Key;
		if (!ensure(IsValid(GraphNodeClass))) continue;

		for (auto&& CountedClass : GraphClassList.Value)
		{
			UClass* NodeClass = CountedClass.Key;
			if (!ensure(IsValid(NodeClass))) continue;

			if (!Iter(GraphNodeClass, NodeClass))
			{
				return;
			}
		}
	}
}

TArray<FString> UHeartGraphNodeRegistry::GetNodeCategories() const
{
	TSet<FString> UnsortedCategories;

	for (auto&& GraphClassList : GraphClasses)
	{
		if (auto&& GraphNodeCDO = GraphClassList.Key->GetDefaultObject<UHeartGraphNode>())
		{
			for (auto&& NodeClass : GraphClassList.Value)
			{
				if (auto&& NodeClassCDO = NodeClass.Key->GetDefaultObject())
				{
					UnsortedCategories.Emplace(GraphNodeCDO->GetNodeCategory(NodeClassCDO).ToString());
				}
			}
		}
	}

	TArray<FString> SortedCategories = UnsortedCategories.Array();
	SortedCategories.Sort();
	return SortedCategories;
}

void UHeartGraphNodeRegistry::GetNodeClasses(TArray<UClass*>& OutClasses) const
{
	for (auto&& GraphClassList : GraphClasses)
	{
		// @todo AHH TObjectPtr drives me nuts!!
		TArray<TObjectPtr<UClass>> ClassObjectPtrArray;
		GraphClassList.Value.GetKeys(ClassObjectPtrArray);
		OutClasses.Append(ClassObjectPtrArray);
	}
}

void UHeartGraphNodeRegistry::GetNodeClassesWithGraphClass(
	TMap<UClass*, TSubclassOf<UHeartGraphNode>>& OutClasses) const
{
	for (auto&& GraphClassList : GraphClasses)
	{
		for (auto&& GraphNode : GraphClassList.Value)
		{
			OutClasses.Add(GraphNode.Key, GraphClassList.Key);
		}
	}
}

void UHeartGraphNodeRegistry::GetFilteredNodeClasses(const FNodeClassFilter& Filter, TArray<UClass*>& OutClasses) const
{
	if (!ensure(Filter.IsBound()))
	{
		return;
	}

	for (auto&& GraphClassList : GraphClasses)
	{
		for (auto&& CountedClass : GraphClassList.Value)
		{
			UClass* Class = CountedClass.Key;
			if (!ensure(IsValid(Class))) continue;

			if (Filter.Execute(Class))
			{
				OutClasses.Add(Class);
			}
		}
	}
}

void UHeartGraphNodeRegistry::GetFilteredNodeClassesWithGraphClass(const FNodeClassFilter& Filter,
	TMap<UClass*, TSubclassOf<UHeartGraphNode>>& OutClasses) const
{
	if (!ensure(Filter.IsBound()))
	{
		return;
	}

	for (auto&& GraphClassList : GraphClasses)
	{
		for (auto&& CountedClass : GraphClassList.Value)
		{
			UClass* Class = CountedClass.Key;
			if (!ensure(IsValid(Class))) continue;

			if (Filter.Execute(Class))
			{
				OutClasses.Add(Class, GraphClassList.Key);
			}
		}
	}
}

TSubclassOf<UHeartGraphNode> UHeartGraphNodeRegistry::GetGraphNodeClassForNode(const UClass* NodeClass) const
{
	for (auto&& Class = NodeClass; Class && Class != UObject::StaticClass(); Class = Class->GetSuperClass())
	{
		for (auto&& ClassList : GraphClasses)
		{
			if (ClassList.Value.Contains(NodeClass))
			{
				return ClassList.Key;
			}
		}
	}

	return nullptr;
}

UClass* UHeartGraphNodeRegistry::GetVisualizerClassForGraphNode(const TSubclassOf<UHeartGraphNode> GraphNodeClass, UClass* VisualizerBase) const
{
	for (UClass* Class = GraphNodeClass; Class && Class != UObject::StaticClass(); Class = Class->GetSuperClass())
	{
		if (auto&& ClassMap = NodeVisualizerMap.Find(Class))
		{
			for (auto&& CountedClass : *ClassMap)
			{
				if (!IsValid(VisualizerBase))
				{
					return CountedClass.Key;
				}

				if (CountedClass.Key->IsChildOf(VisualizerBase))
				{
					return CountedClass.Key;
				}
			}
		}
	}

	// Try and retrieve a fallback visualizer
	if (auto&& Subsystem = GEngine->GetEngineSubsystem<UHeartRegistryRuntimeSubsystem>())
	{
		if (auto&& Fallback = Subsystem->GetFallbackRegistrar())
		{
			for (auto&& FallbackClass : Fallback->Registration.NodeVisualizerClasses)
			{
				ensure(IsValid(FallbackClass));

				if (!IsValid(VisualizerBase) || FallbackClass->IsChildOf(VisualizerBase))
				{
					return FallbackClass;
				}
			}
		}
	}

	UE_LOG(LogHeartNodeRegistry, Warning, TEXT("Registry was unable to find a node visualizer for class '%s'"), *GraphNodeClass->GetName())

	return nullptr;
}

UClass* UHeartGraphNodeRegistry::GetVisualizerClassForGraphPin(const FHeartGraphPinTag GraphPinTag, UClass* VisualizerBase) const
{
	for (FHeartGraphPinTag Tag = GraphPinTag; Tag.IsValid() && Tag != FHeartGraphPinTag::GetRootTag();
			Tag = FHeartGraphPinTag::TryConvert(Tag.RequestDirectParent()))
	{
		if (auto&& ClassMap = PinVisualizerMap.Find(Tag))
		{
			for (auto&& CountedClass : *ClassMap)
			{
				if (!IsValid(VisualizerBase))
				{
					return CountedClass.Key;
				}

				if (CountedClass.Key->IsChildOf(VisualizerBase))
				{
					return CountedClass.Key;
				}
			}
		}
	}

	// Try and retrieve a fallback visualizer
	if (auto&& Subsystem = GEngine->GetEngineSubsystem<UHeartRegistryRuntimeSubsystem>())
	{
		if (auto&& Fallback = Subsystem->GetFallbackRegistrar())
		{
			for (auto&& FallbackClass : Fallback->Registration.PinVisualizerClasses)
			{
				ensure(IsValid(FallbackClass));

				if (!IsValid(VisualizerBase) || FallbackClass->IsChildOf(VisualizerBase))
				{
					return FallbackClass;
				}
			}
		}
	}

	UE_LOG(LogHeartNodeRegistry, Warning, TEXT("Registry was unable to find a pin visualizer for Tag '%s'"), *GraphPinTag.GetTagName().ToString())

	return nullptr;
}

UClass* UHeartGraphNodeRegistry::GetVisualizerClassForGraphConnection(const FHeartGraphPinTag FromPinTag,
																	  const FHeartGraphPinTag ToPinTag,
																	  UClass* VisualizerBase) const
{
	// @todo add ability to override the connection class to anything other than the default

	// Try and retrieve a fallback visualizer
	if (auto&& Subsystem = GEngine->GetEngineSubsystem<UHeartRegistryRuntimeSubsystem>())
	{
		if (auto&& Fallback = Subsystem->GetFallbackRegistrar())
		{
			for (auto&& FallbackClass : Fallback->Registration.ConnectionVisualizerClasses)
			{
				ensure(IsValid(FallbackClass));

				if (!IsValid(VisualizerBase) || FallbackClass->IsChildOf(VisualizerBase))
				{
					return FallbackClass;
				}
			}
		}
	}

	UE_LOG(LogHeartNodeRegistry, Warning, TEXT("Registry was unable to find a connection visualizer from Tag '%s' to Tag '%s'"),
											 *FromPinTag.GetTagName().ToString(), *ToPinTag.GetTagName().ToString())

	return nullptr;
}

void UHeartGraphNodeRegistry::AddRegistrar(UGraphNodeRegistrar* Registrar)
{
	if (!ensure(IsValid(Registrar)))
	{
		return;
	}

	// Only allow registry once
	if (ContainedRegistrars.Contains(Registrar))
	{
		// We really can't warn against this, since the editor tries to re-add everything occasionally
		//UE_LOG(LogHeartNodeRegistry, Warning, TEXT("Tried to add Registrar that was already registered!"));
		return;
	}

	AddRegistrationList(Registrar->Registration);

	ContainedRegistrars.Add(Registrar);
}

void UHeartGraphNodeRegistry::RemoveRegistrar(UGraphNodeRegistrar* Registrar)
{
	if (!ensure(IsValid(Registrar)))
	{
		return;
	}

	if (!ContainedRegistrars.Contains(Registrar))
	{
		// We really can't warn against this, since the registrars try to remove themselves precautionarily
		//UE_LOG(LogHeartNodeRegistry, Warning, TEXT("Tried to remove Registrar that wasn't registered!"));
		return;
	}

	RemoveRegistrationList(Registrar->Registration);

	ContainedRegistrars.Remove(Registrar);
}

void UHeartGraphNodeRegistry::DeregisterAll()
{
	GraphClasses.Empty();
	NodeVisualizerMap.Empty();
	PinVisualizerMap.Empty();
	ContainedRegistrars.Empty();
}
