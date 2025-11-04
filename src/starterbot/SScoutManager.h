#pragma once

#include <BWAPI.h>
#include <vector>
#include <set>
#include <utility>
#include <queue>
#include "MapTools.h"


class SScoutManager
{
	struct EnemyDetails {
		int playerNumber;
		BWAPI::Race race;
		std::vector<BWAPI::TilePosition> enemyBases;
		std::set<BWAPI::UnitType> seenUnitTypes;	//This shouldn't really ever change... 
		std::vector<std::pair<BWAPI::Unit, BWAPI::Position>> trackedUnits;	// An unordered map would probably be best here... but for now will find it in this way
	};

	std::vector<EnemyDetails> enemyData;
	std::vector<BWAPI::TilePosition> allBaseLocations;
	std::vector<BWAPI::TilePosition> ourBases;		//Probably should have an ally base as well
	std::vector<BWAPI::TilePosition> allyBases;		//Probably should have an ally base as well	- this should be easier to update

	std::vector<BWAPI::TilePosition> CheckedSquares;

public:
	MapTools* m_tools;

	void addSightedStructure(BWAPI::TilePosition, BWAPI::Unit, BWAPI::Player);	// Should be a pair probably
	void updateSightedUnit(BWAPI::Position, BWAPI::Unit, BWAPI::Player);		// Add or update - to both the 

	void updateEnemyBase(BWAPI::TilePosition, BWAPI::Player);	//More complex... if enemy base sighted (even if none are) need to check to see if that is new information - can skip for 

	void getAllBasePositions();	//Disregard anything blocking them at the beginning... maybe should note if anything needs to be cleared
	void displayMinPositions();


	bool validBaseLocation(BWAPI::TilePosition);
	void drawBases();

	BWAPI::TilePosition getBaseLocation(BWAPI::Unitset& resources);

	void onStart();
	void onFrame();
};