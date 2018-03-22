// Fill out your copyright notice in the Description page of Project Settings.

#include "BalancedFPSLevelGeneratorTool.h"
#include "MessageDialog.h"
#include "Engine/World.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h "
#include "Editor/UnrealEd/Public/FileHelpers.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Engine/StaticMesh.h"
#include "Engine/Blueprint.h"
// For access to the GEditor object:
#include "Editor/UnrealEd/Public/Editor.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Engine/PointLight.h"
#include "Runtime/Core/Public//Math/UnrealMathUtility.h"
#include "Runtime/Core/Public/HAL/Platform.h"
#include "random"

#include "Zone.h"

// Initialise:
UBalancedFPSLevelGeneratorTool::UBalancedFPSLevelGeneratorTool()
{
	WallPanelBlueprintAsset = ConstructorHelpers::FObjectFinder<UBlueprint>(
		TEXT("Blueprint'/Game/BalancedFPSLevelGeneratorAssets/Blueprints/WallPanel.WallPanel'"))
		.Object;

	for (int ZoneBlueprintCounter = 0; ZoneBlueprintCounter < TOTAL_ZONE_BLUEPRINT_COUNT;
		ZoneBlueprintCounter++)
	{
		FString IncrementalPathString = "Blueprint'/Game/BalancedFPSLevelGeneratorAssets/Blueprints/WangTiles/";
		// As ZoneBlueprintCounter starts at 0, add one to it, when building this FString:
		IncrementalPathString.Append(FString("WangTile") + FString::FromInt(ZoneBlueprintCounter + 1) + FString(".WangTile") +
			FString::FromInt(ZoneBlueprintCounter + 1) + FString("'"));
		LevelZoneTileBlueprints.Add(ConstructorHelpers::FObjectFinder<UBlueprint>(*IncrementalPathString)
			.Object);
	}

	DefaultRelativePanelScale = FVector(1.0f, 1.0f, 1.0f);
	LevelExtents = FVector2D(300.0f, 300.0f);
	LevelGenerationStartPoint = FVector(0.0f, 0.0f, 0.0f);
	VariationProbability = 0.10f;
	RecombinationProbability = 0.750f;
	MutationValue = 1.0f;
	AddMutationProbability = 0.10f;
	MultiplyMutationProbability = 0.750f;

	HelpText = FText::FromString(
		"Change properties, before generating a level" 
		" in this dialog panel.");
}

/**
	Generate the level.
	Using https://docs.unrealengine.com/latest/INT/API/Runtime/Engine/Engine/UWorld/index.html
	for reference, as well as https://answers.unrealengine.com/questions/440347/create-a-new-level-from-c-code.html
*/

void UBalancedFPSLevelGeneratorTool::GenerateLevel()
{
	// Initialise the level generation area first...
	InitialiseLevelGenerationArea();		
}

// These functions handle initialisation of the level generation area:
void UBalancedFPSLevelGeneratorTool::InitialiseLevelGenerationArea()
{
	// Encapsulate this level generation area first...
	EncapsulateLevelGenerationArea();

	// ...add a light source to it...
	AddLightSourceToLevelGenerationArea();

	// ...then the level Zones can be added to it:
	AddZonesToLevelGenerationArea();
}

void UBalancedFPSLevelGeneratorTool::EncapsulateLevelGenerationArea()
{
	// For each wall panel to use in initialisation:
	static AActor* WallPanelActor;

	// For testing:
	int FrontFaceLoopCycleCount = 0;
	int RightFaceLoopCycleCount = 0;
	int BackFaceLoopCycleCount = 0;
	int LeftFaceLoopCycleCount = 0;

	
	// Offset by DEFAULT_TILE_DEPTH, to allow Zones to fill in these gaps:
	for (float CurrentZPosition = 0; CurrentZPosition < DEFAULT_TILE_HEIGHT;
		CurrentZPosition += DEFAULT_TILE_HEIGHT)
	{
		// Front face:
		for (float CurrentXPosition = LevelGenerationStartPoint.X;
			CurrentXPosition < LevelExtents.X; CurrentXPosition +=
			DEFAULT_TILE_WIDTH)
		{
			FVector CurrentPosition = FVector(CurrentXPosition, 
				LevelGenerationStartPoint.Y - DEFAULT_TILE_DEPTH, CurrentZPosition
				- DEFAULT_TILE_DEPTH);
			FRotator FrontFaceRotation = FRotator::ZeroRotator;

			FTransform LevelPanelTransform = FTransform(FrontFaceRotation.Quaternion(), CurrentPosition, DefaultRelativePanelScale);
			
			// *GEditor->GetEditorWorldContext().ExternalReferences[0] just to get the world context...
			WallPanelActor = UGameplayStatics::BeginSpawningActorFromBlueprint(GEditor->GetEditorWorldContext().World()->GetCurrentLevel(),
				WallPanelBlueprintAsset, LevelPanelTransform, false);
			WallPanelActor->ExecuteConstruction(LevelPanelTransform, nullptr, nullptr, true);
			FrontFaceLoopCycleCount++;
			//UGameplayStatics::FinishSpawningActor(WallPanelActor, LevelPanelTransform);
		}

		// Right face:
		for (float CurrentYPosition = LevelGenerationStartPoint.Y;
			CurrentYPosition < LevelExtents.Y; CurrentYPosition +=
			DEFAULT_TILE_WIDTH)
		{
			FVector CurrentPosition = FVector(LevelGenerationStartPoint.X + LevelExtents.X + DEFAULT_TILE_DEPTH,
				CurrentYPosition, CurrentZPosition - DEFAULT_TILE_DEPTH);
			FRotator RightFaceRotation = FRotator(0.0f, 90.0f, 0.0f);

			FTransform LevelPanelTransform = FTransform(RightFaceRotation.Quaternion(), CurrentPosition, DefaultRelativePanelScale);

			// *GEditor->GetEditorWorldContext().ExternalReferences[0] just to get the world context...
			WallPanelActor = UGameplayStatics::BeginSpawningActorFromBlueprint(GEditor->GetEditorWorldContext().World()->GetCurrentLevel(),
				WallPanelBlueprintAsset, LevelPanelTransform, false);
			WallPanelActor->ExecuteConstruction(LevelPanelTransform, nullptr, nullptr, true);
			RightFaceLoopCycleCount++;
		}

		// Back face:
		for (float CurrentXPosition = LevelGenerationStartPoint.X + LevelExtents.X;
			CurrentXPosition > LevelGenerationStartPoint.X; CurrentXPosition -=
			DEFAULT_TILE_WIDTH)
		{
			FVector CurrentPosition = FVector(CurrentXPosition,
				LevelGenerationStartPoint.Y + LevelExtents.Y + DEFAULT_TILE_DEPTH,
				CurrentZPosition - DEFAULT_TILE_DEPTH);
			FRotator BackFaceRotation = FRotator(0.0f, 180.0f, 0.0f);

			FTransform LevelPanelTransform = FTransform(BackFaceRotation.Quaternion(), CurrentPosition, DefaultRelativePanelScale);

			// *GEditor->GetEditorWorldContext().ExternalReferences[0] just to get the world context...
			WallPanelActor = UGameplayStatics::BeginSpawningActorFromBlueprint(GEditor->GetEditorWorldContext().World()->GetCurrentLevel(),
				WallPanelBlueprintAsset, LevelPanelTransform, false);
			WallPanelActor->ExecuteConstruction(LevelPanelTransform, nullptr, nullptr, true);
			BackFaceLoopCycleCount++;
		}

		// Left face:
		for (float CurrentYPosition = LevelGenerationStartPoint.Y + LevelExtents.Y;
			CurrentYPosition > LevelGenerationStartPoint.Y; CurrentYPosition -=
			DEFAULT_TILE_WIDTH)
		{
			FVector CurrentPosition = FVector(LevelGenerationStartPoint.X - DEFAULT_TILE_DEPTH,
				CurrentYPosition, CurrentZPosition - DEFAULT_TILE_DEPTH);
			FRotator FrontFaceRotation = FRotator(0.0f, -90.0f, 0.0f);

			FTransform LevelPanelTransform = FTransform(FrontFaceRotation.Quaternion(), CurrentPosition, DefaultRelativePanelScale);

			// *GEditor->GetEditorWorldContext().ExternalReferences[0] just to get the world context...
			WallPanelActor = UGameplayStatics::BeginSpawningActorFromBlueprint(GEditor->GetEditorWorldContext().World()->GetCurrentLevel(),
				WallPanelBlueprintAsset, LevelPanelTransform, false);
			WallPanelActor->ExecuteConstruction(LevelPanelTransform, nullptr, nullptr, true);
			LeftFaceLoopCycleCount++;
		}

		// Top and bottom faces:
		for (float CurrentYPosition = LevelGenerationStartPoint.Y;
			CurrentYPosition < LevelExtents.Y; CurrentYPosition +=
			DEFAULT_TILE_WIDTH)
		{			
			for (float CurrentXPosition = LevelGenerationStartPoint.X;
				CurrentXPosition < LevelExtents.X; CurrentXPosition +=
				DEFAULT_TILE_WIDTH)
			{
				FVector CurrentPosition = FVector(CurrentXPosition,
					CurrentYPosition, CurrentZPosition - DEFAULT_TILE_DEPTH);
				// It seems rotation has been shuffled to the left, with a 
				// wrap around (Z for Y, Y for X etc.) in UE4:
				FRotator TopBottomFaceRotation = FRotator(0.0f, 0.0f, 90.0f);

				FTransform LevelPanelTransform = FTransform(TopBottomFaceRotation.Quaternion(), CurrentPosition, DefaultRelativePanelScale);

				// For the bottom tile first...
				WallPanelActor = UGameplayStatics::BeginSpawningActorFromBlueprint(GEditor->GetEditorWorldContext().World()->GetCurrentLevel(),
					WallPanelBlueprintAsset, LevelPanelTransform, false);
				WallPanelActor->ExecuteConstruction(LevelPanelTransform, nullptr, nullptr, true);
				// ...then the top tile:
				FVector TopTilePosition = FVector(CurrentXPosition, CurrentYPosition, CurrentZPosition + DEFAULT_TILE_HEIGHT + DEFAULT_TILE_DEPTH);
				LevelPanelTransform.SetComponents(LevelPanelTransform.GetRotation(), TopTilePosition, LevelPanelTransform.GetScale3D());
				WallPanelActor = UGameplayStatics::BeginSpawningActorFromBlueprint(*GEditor->GetEditorWorldContext().ExternalReferences[0],
					WallPanelBlueprintAsset, LevelPanelTransform, false);
				WallPanelActor->ExecuteConstruction(LevelPanelTransform, nullptr, nullptr, true);
			}
		}
	}
}

void UBalancedFPSLevelGeneratorTool::AddLightSourceToLevelGenerationArea()
{
	// Put a point-light at the centre of the now encapsulated level generation area:
	FTransform DefaultLightSourceTransform = FTransform(FRotator::ZeroRotator.Quaternion(), FVector(LevelGenerationStartPoint.X + 0.50f
		* LevelExtents.X, LevelGenerationStartPoint.Y + 0.50f * LevelExtents.Y, LevelGenerationStartPoint.Z + 0.50f * DEFAULT_TILE_HEIGHT),
		FVector(1.0f));
	APointLight* DefaultLightSource = Cast<APointLight>(GEditor->AddActor(GEditor->GetEditorWorldContext().World()->GetCurrentLevel(),
		APointLight::StaticClass(), DefaultLightSourceTransform));
}

// Now zones can be added to it (Wang Tiles):
void UBalancedFPSLevelGeneratorTool::AddZonesToLevelGenerationArea()
{
	// For each Zone to use in initialisation:
	static AActor* ZoneTile;
	FVector DEFAULT_ZONE_SCALE = FVector(1.0f, 1.0f, 1.0f);

	// To offset the placement of zones (to fall within the level generation area):
	const FVector2D ZONE_POSITION_OFFSET = FVector2D(50.0f, 50.0f);
	
	// For the Zone Blueprints, as Actors:
	TArray<AActor*> ActorZones;
	UGameplayStatics::GetAllActorsOfClass(GEditor->GetEditorWorldContext()
		.World()->GetCurrentLevel(), AZone::StaticClass(), ActorZones);
	
	// To store the array of zones
	TArray<AZone*> LevelZones;
	for (int ActorZonesCounter = 0; ActorZonesCounter < ActorZones.Num(); ActorZonesCounter++)
	{
		// Initialise here as well:
		LevelZones.Add(Cast<AZone, AActor>(ActorZones[ActorZonesCounter]));
		LevelZones[ActorZonesCounter]->InitialiseZone();
		LevelZones[ActorZonesCounter]->UpdateEdges();
	}

	// The main loop to place the zones:
	for (float CurrentZPosition = LevelGenerationStartPoint.Z;
		CurrentZPosition < DEFAULT_TILE_HEIGHT; CurrentZPosition +=
		DEFAULT_TILE_HEIGHT)
	{
		// Work backwards from the last row:
		for (float CurrentYPosition = LevelGenerationStartPoint.Y + LevelExtents.Y;
			CurrentYPosition > LevelGenerationStartPoint.Y; CurrentYPosition -=
			DEFAULT_TILE_WIDTH)
		{
			for (float CurrentXPosition = LevelGenerationStartPoint.X;
				CurrentXPosition < LevelExtents.X; CurrentXPosition +=
				DEFAULT_TILE_WIDTH)
			{
				FVector CurrentPosition = FVector(CurrentXPosition,
					CurrentYPosition, CurrentZPosition);
				FRotator ZoneRotation = FRotator(0.0f, 0.0f, 0.0f);

				FTransform LevelZoneTransform = FTransform(ZoneRotation.Quaternion(), CurrentPosition, DEFAULT_ZONE_SCALE);
				
				// Offset the X and Y components of the CurrentPosition:
				LevelZoneTransform.SetComponents(LevelZoneTransform.GetRotation(), FVector(LevelZoneTransform.GetLocation().X + 
					ZONE_POSITION_OFFSET.X, LevelZoneTransform.GetLocation().Y - ZONE_POSITION_OFFSET.Y, LevelZoneTransform.GetLocation().Z),
					LevelZoneTransform.GetScale3D());

				ZoneTile = UGameplayStatics::BeginSpawningActorFromBlueprint(GEditor->GetEditorWorldContext().World()->GetCurrentLevel(),
					GetSuitableZoneTile(FVector2D(CurrentPosition), LevelZones), LevelZoneTransform, false);
				// Sanity check:
				if (ZoneTile)
				{
					ZoneTile->ExecuteConstruction(LevelZoneTransform, nullptr, nullptr, true);
				}				
			}
		}	
	}
}

UBlueprint* UBalancedFPSLevelGeneratorTool::GetSuitableZoneTile(FVector2D CurrentPlacementPosition, TArray<AZone*> LevelZoneTiles)
{
	/**
	* The 'edge' colours to match a Zone against, to find a suitable Zone for this CurrentPlacementPosition.
	* Area 'edge' order:
	* First element: North 'edge'.
	* Second element: East 'edge'.
	* Third element: South 'edge'.
	* Fourth element: West 'edge'.
	*/
	std::vector<AZone::EdgeColour> TargetEdgeColours = std::vector<AZone::EdgeColour>(size_t(4));

	// USE THE STD LIB RANDOM NUMBER GENERATION STREAM FGIahwIGHWAPU

	// The Zone sub-set to choose a Zone-tile from:
	TArray<AZone*> ZoneSubSet;
	// Make sure the sub-set is clear (so as to not chooose an invalid zone):
	ZoneSubSet.Empty();

	// If this Zone is to be placed against an edge of
	// the level generation area, then the tile will 
	// have to match 'pure' wall 'edges', as well as
	// the Edge colour of any other adjacent Zones:

	// South level-generation area 'edge':
	if (CurrentPlacementPosition.Y == LevelGenerationStartPoint.Y)
	{
		TargetEdgeColours[2] = AZone::EdgeColour::Red;
	}

	// North level-generation area 'edge':
	if (CurrentPlacementPosition.Y == LevelExtents.Y)
	{
		TargetEdgeColours[0] = AZone::EdgeColour::Red;
	}

	// West level-generation area 'edge':
	if (CurrentPlacementPosition.X == LevelGenerationStartPoint.X)
	{
		TargetEdgeColours[3] = AZone::EdgeColour::Red;
	}

	// East level-generation area 'edge':
	if (CurrentPlacementPosition.X == LevelExtents.X)
	{
		TargetEdgeColours[1] = AZone::EdgeColour::Red;
	}

	// Check through PlacedLevelZones, to complete TargetEdgeColours.
	// (If any Zones have been placed):
	for (int PlacedZonesIterator = 0; PlacedZonesIterator < PlacedLevelZones.Num();
		PlacedZonesIterator++)
	{
		if (PlacedLevelZones.Num() == 0)
		{
			// No zones have been placed:
			break;
		}

		// Get the Edge colours of this Zone:
		std::vector<AZone::EdgeColour> PlacedEdgeColours = PlacedLevelZones[PlacedZonesIterator]->GetEdgeColours();

		// There is a tile to the west:
		if (PlacedLevelZones[PlacedZonesIterator]->GetActorLocation().X ==
			(CurrentPlacementPosition.X - DEFAULT_TILE_WIDTH))
		{
			// To make sure the Edges are up to date:
			PlacedLevelZones[PlacedZonesIterator]->UpdateEdges();
			PlacedLevelZones[PlacedZonesIterator]->UpdateZoneEdgesColour();
			TargetEdgeColours[3] = PlacedEdgeColours[3];
		}

		// There is a tile to the south:
		if (PlacedLevelZones[PlacedZonesIterator]->GetActorLocation().Y ==
			(CurrentPlacementPosition.Y - DEFAULT_TILE_HEIGHT))
		{
			// To make sure the Edges are up to date:
			PlacedLevelZones[PlacedZonesIterator]->UpdateEdges();
			PlacedLevelZones[PlacedZonesIterator]->UpdateZoneEdgesColour();
			TargetEdgeColours[2] = PlacedEdgeColours[2];
		}
	}

	// Get a set of Zones that match the TargetEdgeColours...
	for (int ZoneSetIterator = 0; ZoneSetIterator < LevelZoneTiles.Num(); ZoneSetIterator++)
	{
		// For the Edge-colours of this current Zone in the set:
		LevelZoneTiles[ZoneSetIterator]->UpdateZoneEdgesColour();
		std::vector<AZone::EdgeColour> CurrentZoneEdgeColours = LevelZoneTiles[ZoneSetIterator]->GetEdgeColours();

		// For validating if a tile matches up to the TargetEdgeColours:
		bool ZoneMatchesToTargetEdgeColours = true;

		// To check each EdgeColour against CurrentZoneEdgeColours:
		for (int TargetEdgeColoursIterator = 0; TargetEdgeColoursIterator < TargetEdgeColours.size();
			TargetEdgeColoursIterator++)
		{
			if (TargetEdgeColours[TargetEdgeColoursIterator] > AZone::EdgeColour::Grey - 1)
			{
				// This CurrentEdgeColour would not match up to TargetEdgeColour, so this zone would not go
				// in the CurrentPlacementPosition.
				// For the mean time, grey is assumed to be colourless (so matching with any colour):
				if (!(CurrentZoneEdgeColours[TargetEdgeColoursIterator] == TargetEdgeColours[TargetEdgeColoursIterator]) &&
					TargetEdgeColours[TargetEdgeColoursIterator] != AZone::EdgeColour::Grey)
				{
					ZoneMatchesToTargetEdgeColours = false;
					break;
				}
			}
		}

		if (ZoneMatchesToTargetEdgeColours)
		{
			ZoneSubSet.Add(LevelZoneTiles[ZoneSetIterator]);
		}
	}

	// ...then pick one of these Zones from the sub-set:

	// If there are any Zones in the sub-set:
	if (ZoneSubSet.Num() == 0)
	{
		return nullptr;
	}

	// For the chosen Zone-index (pseudo-randomly chosen):	
	FMath::RandInit(time_t(null));
	FMath::Rand(); // Flush the rand stream.
	int ZoneChoice = FMath::RandRange(0, ZoneSubSet.Num() - 1);
	
	// Return the Blueprint that represents this Zone:
	for (int ZoneBlueprintIterator = 0; ZoneBlueprintIterator < LevelZoneTiles.Num(); ZoneBlueprintIterator++)
	{
		if (ZoneSubSet[ZoneChoice]->GetName() == LevelZoneTiles[ZoneBlueprintIterator]->GetName())
		{
			PlacedLevelZones.Add(ZoneSubSet[ZoneChoice]);
			return LevelZoneTileBlueprints[ZoneBlueprintIterator];
		}
	}
	
	return nullptr;
}
