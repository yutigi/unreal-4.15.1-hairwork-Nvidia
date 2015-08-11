// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "EnginePrivate.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphSchema.h"
#include "BlueprintUtilities.h"
#if WITH_EDITOR
#include "Editor/UnrealEd/Public/Kismet2/BlueprintEditorUtils.h"
#include "SlateBasics.h"
#include "ScopedTransaction.h"
#include "Editor/UnrealEd/Public/Kismet2/Kismet2NameValidators.h"
#endif

#define LOCTEXT_NAMESPACE "EdGraph"

DEFINE_LOG_CATEGORY_STATIC(LogEdGraph, Log, All);

/////////////////////////////////////////////////////
// FGraphReference

void FGraphReference::PostSerialize(const FArchive& Ar)
{
#if WITH_EDITORONLY_DATA
	if (Ar.UE4Ver() >= VER_UE4_K2NODE_REFERENCEGUIDS)
	{
		// Because the macro instance could have been saved with a GUID that was allocated 
		// but the macro graph never actually saved with that value we are forced to make 
		// sure to refresh the GUID and make sure it is up to date
		if (MacroGraph)
		{
			GraphGuid = MacroGraph->GraphGuid;
		}
	}
#endif
}

#if WITH_EDITORONLY_DATA
void FGraphReference::SetGraph(UEdGraph* InGraph)
{
	MacroGraph = InGraph;
	if (InGraph)
	{
		GraphBlueprint = InGraph->GetTypedOuter<UBlueprint>();
		GraphGuid = InGraph->GraphGuid;
	}
	else
	{
		GraphBlueprint = NULL;
		GraphGuid.Invalidate();
	}
}

UEdGraph* FGraphReference::GetGraph() const
{
	if (MacroGraph == NULL)
	{
		if (GraphBlueprint)
		{
			TArray<UObject*> ObjectsInPackage;
			GetObjectsWithOuter(GraphBlueprint, ObjectsInPackage);

			for (int32 Index = 0; Index < ObjectsInPackage.Num(); ++Index)
			{
				UEdGraph* FoundGraph = Cast<UEdGraph>(ObjectsInPackage[Index]);
				if (FoundGraph && FoundGraph->GraphGuid == GraphGuid)
				{
					MacroGraph = FoundGraph;
					break;
				}
			}
		}
	}

	return MacroGraph;
}
#endif

/////////////////////////////////////////////////////
// UEdGraph

UEdGraph::UEdGraph(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bEditable = true;
	bAllowDeletion = true;
}

#if WITH_EDITORONLY_DATA
void UEdGraph::PostInitProperties()
{
	Super::PostInitProperties();

	if (!IsTemplate())
	{
		GraphGuid = FGuid::NewGuid();
	}
}

void UEdGraph::PostLoad()
{
	Super::PostLoad();

	// Strip out null nodes (likely from missing node classes) as they will cause crashes 
	for (int32 i = Nodes.Num() - 1; i >= 0; i--)
	{
		if (Nodes[i] == nullptr)
		{
			Nodes.RemoveAt(i);
			UE_LOG(LogBlueprint, Warning, TEXT("Found NULL Node in EdGraph Nodes array. A node type may have been deleted without creating an ActiveClassRedictor to K2Node_DeadClass."));
		}
	}
}
#endif

const UEdGraphSchema* UEdGraph::GetSchema() const
{
	if (Schema == NULL)
	{
		return NULL;
	}
	return GetDefault<UEdGraphSchema>(Schema);
}

FDelegateHandle UEdGraph::AddOnGraphChangedHandler( const FOnGraphChanged::FDelegate& InHandler )
{
	return OnGraphChanged.Add( InHandler );
}

void UEdGraph::RemoveOnGraphChangedHandler( FDelegateHandle Handle )
{
	OnGraphChanged.Remove( Handle );
}

UEdGraphNode* UEdGraph::CreateNode( TSubclassOf<UEdGraphNode> NewNodeClass, bool bSelectNewNode/* = true*/ )
{
	UEdGraphNode* NewNode = NewObject<UEdGraphNode>(this, NewNodeClass, NAME_None, RF_Transactional);

	if (HasAnyFlags(RF_Transient))
	{
		NewNode->SetFlags(RF_Transient);
	}

	AddNode(NewNode, false, bSelectNewNode );
	return NewNode;
}

void UEdGraph::AddNode( UEdGraphNode* NodeToAdd, bool bFromUI/* = false*/, bool bSelectNewNode/* = true*/ )
{
	this->Nodes.Add(NodeToAdd);
	check(NodeToAdd->GetOuter() == this);

	// Create the graph
	EEdGraphActionType AddNodeAction = GRAPHACTION_AddNode;

	if (bSelectNewNode)
	{
		AddNodeAction = (EEdGraphActionType)( ((int32)AddNodeAction) | GRAPHACTION_SelectNode );
	}

	FEdGraphEditAction Action(AddNodeAction, this, NodeToAdd, bFromUI);
	
	NotifyGraphChanged( Action );
}

void UEdGraph::SelectNodeSet(TSet<const UEdGraphNode*> NodeSelection, bool bFromUI/*= false*/) 
{
	FEdGraphEditAction SelectionAction;

	SelectionAction.Action = GRAPHACTION_SelectNode;

	SelectionAction.Graph = this;
	SelectionAction.Nodes = NodeSelection;

	NotifyGraphChanged(SelectionAction);
}

bool UEdGraph::RemoveNode( UEdGraphNode* NodeToRemove )
{
	Modify();

	int32 NumTimesNodeRemoved = Nodes.Remove(NodeToRemove);
#if WITH_EDITOR
	NodeToRemove->BreakAllNodeLinks();
#endif	//#if WITH_EDITOR

	FEdGraphEditAction RemovalAction;
	RemovalAction.Graph = this;
	RemovalAction.Action = GRAPHACTION_RemoveNode;
	RemovalAction.Nodes.Add(NodeToRemove);
	NotifyGraphChanged(RemovalAction);

	return NumTimesNodeRemoved > 0;
}

void UEdGraph::NotifyGraphChanged()
{
	FEdGraphEditAction Action;
	OnGraphChanged.Broadcast(Action);
}

void UEdGraph::NotifyGraphChanged(const FEdGraphEditAction& InAction)
{
	OnGraphChanged.Broadcast(InAction);
}

void UEdGraph::MoveNodesToAnotherGraph(UEdGraph* DestinationGraph, bool bIsLoading, bool bInIsCompiling/* = false*/)
{
	// Move one node over at a time
	DestinationGraph->Nodes.Reserve(DestinationGraph->Nodes.Num() + Nodes.Num());
	while (Nodes.Num())
	{
		if (UEdGraphNode* Node = Nodes.Pop(/*bAllowShrinking=*/ false))
		{
#if WITH_EDITOR
			// During compilation, do not move ghost nodes, they are not used during compilation.
			if (bInIsCompiling && !Node->bIsNodeEnabled)
			{
				// Break all node links, if any exist, do not move the node
				Node->BreakAllNodeLinks();
				continue;
			}
#endif

			// Let the name be autogenerated, to automatically avoid naming conflicts
			// Since this graph is always going to come from a cloned source graph, user readable names can come from the remap stored in a MessageLog.
	
			//@todo:  The bIsLoading check is to force no reset loaders when blueprints are compiling on load.  This might not catch all cases though!
			Node->Rename(/*NewName=*/ NULL, /*NewOuter=*/ DestinationGraph, REN_DontCreateRedirectors | (bIsLoading ? REN_ForceNoResetLoaders : 0));
	
			DestinationGraph->Nodes.Add(Node);
		}
	}

	DestinationGraph->NotifyGraphChanged();
	NotifyGraphChanged();
}

void UEdGraph::GetAllChildrenGraphs(TArray<UEdGraph*>& Graphs) const
{
#if WITH_EDITORONLY_DATA
	for (int32 i = 0; i < SubGraphs.Num(); ++i)
	{
		UEdGraph* Graph = SubGraphs[i];
		if (ensureMsgf(Graph, TEXT("%s has invalid SubGraph array entry at %d"), *GetFullName(), i))
		{
			Graphs.Add(Graph);
			Graph->GetAllChildrenGraphs(Graphs);
		}
	}
#endif // WITH_EDITORONLY_DATA
}

FVector2D UEdGraph::GetGoodPlaceForNewNode()
{
	FVector2D BottomLeft(0,0);

	if(Nodes.Num() > 0)
	{
		UEdGraphNode* Node = Nodes[0];
		BottomLeft = FVector2D(Node->NodePosX, Node->NodePosY);
		for(int32 i=1; i<Nodes.Num(); i++)
		{
			Node = Nodes[i];
			if ( Node )
			{
				BottomLeft.X = FMath::Min<float>(BottomLeft.X, Node->NodePosX);
				BottomLeft.Y = FMath::Max<float>(BottomLeft.Y, Node->NodePosY);
			}
		}
	}

	return BottomLeft + FVector2D(0, 256);
}

#if WITH_EDITOR

void UEdGraph::NotifyPreChange( const FString& PropertyName )
{
	// no notification is hooked up yet
}

void UEdGraph::NotifyPostChange( const FPropertyChangedEvent& PropertyChangedEvent, const FString& PropertyName )
{
#if WITH_EDITORONLY_DATA
	PropertyChangedNotifiers.Broadcast(PropertyChangedEvent, PropertyName);
#endif
}

FDelegateHandle UEdGraph::AddPropertyChangedNotifier(const FOnPropertyChanged::FDelegate& InDelegate )
{
#if WITH_EDITORONLY_DATA
	return PropertyChangedNotifiers.Add(InDelegate);
#else
	return FDelegateHandle();
#endif
}

void UEdGraph::RemovePropertyChangedNotifier(FDelegateHandle Handle )
{
#if WITH_EDITORONLY_DATA
	PropertyChangedNotifiers.Remove(Handle);
#endif
}
#endif	//WITH_EDITOR

/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
