#pragma once

#include <BWAPI.h>
#include <Filter.h>
#include <vector>
#include "Tools.h"

class SUnitManager {
	BWAPI::Unitset Scouting;	// I'll probably eventually change these to be seperate classes
	BWAPI::Unitset MainFight;
	BWAPI::Unitset Harass;
	BWAPI::Unitset Workers;
	BWAPI::Unitset Overlords;
	BWAPI::Unitset Hatcheries;	//Include Lair and Hive
	BWAPI::Unitset Unsorted;	// Place Units that need to be sorted
	// Should hook the Build Manager up to this to ask for workers and the like

	struct ResourceWorkers{
		BWAPI::Unit Resource;
		BWAPI::Unitset Workers;
	};	// Used for optimal mining
	std::vector<ResourceWorkers> MiningTracker;

	// Eventually will need to have a way to get reference to state of game and use it in the sorting
	bool showDebug = false;

public:
	void toggleDebug();

	BWAPI::Unit GetWorkerNearPosition(BWAPI::Position, bool remove=true);
	BWAPI::Unit GetWorkerNearPosition(BWAPI::TilePosition, bool remove=true);	//Have this just call the above after converting
	BWAPI::Unit GetScout();						// Adds unit to Scouting - first search the main fight, then if have overlord speed - one from here - then a worker otherwise
	void RemoveUnitFromUnitsets(BWAPI::Unit);	// Should be able to remove from any group - including MiningTracker

	void GatherAndAttack();						// Simple attack for starters
	void PlaceUnitCreated(BWAPI::Unit);			// Place the newly created unit in the unitset - don't add larva

	void SortUnsorted();						// Place unsorted in appropriate unitset with the given state

	bool isBase(BWAPI::Unit u);

	void MicroWorkerMining(BWAPI::Unit, BWAPI::Unit);	//Give the Resource and the Worker
	void Micro();

	void listResourceWorkers();

	void AddResourcesToMiningTracker();		// When to call this, not sure (not too often hopefully) - maybe just in base created? (And have it check every hatchery and the like?)
	bool ResourceInMiningTracker(BWAPI::Unit Resource);
	int ResourceIndexInMining(BWAPI::Unit Resource);

	void ResourceLost(BWAPI::Unit FieldLost);	// Remove the mineral/gas from resources, and send workers in that group back in Unsorted (May need to change how it works with refinery...)
	void BaseLost(BWAPI::Unit killedBase);			// Remove the resources near that base if there isn't another hatchery nearby
	void BaseCreated(BWAPI::Unit base);		// Add resources if they don't already exist for tracking of mining

	void onStart();
	void onFrame();
};