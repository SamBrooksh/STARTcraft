#pragma once

#include <BWAPI.h>

class SUnitManager {
	BWAPI::Unitset Scouting;	//I'll probably eventually change these to be seperate classes
	BWAPI::Unitset MainFight;
	BWAPI::Unitset Harass;
	BWAPI::Unitset Workers;
	// Should hook the Build Manager up to this to ask for workers and the like

public:
	BWAPI::Unit GetWorkerNearPosition(BWAPI::Position);
	BWAPI::Unit GetScout();		// Adds unit to Scouting
	void RemoveUnitFromUnitsets(BWAPI::Unit);

	void GatherAndAttack();		// Simple attack for starters
	void PlaceUnitCreated(BWAPI::Unit);	// Place the newly created unit in the unitset - don't add larva


};