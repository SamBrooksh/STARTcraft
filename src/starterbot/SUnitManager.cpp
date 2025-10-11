#include "SUnitManager.h"

void SUnitManager::GatherAndAttack()
{
	BWAPI::Player self = BWAPI::Broodwar->self();
	BWAPI::TilePosition myStart = self->getStartLocation();
	BWAPI::Position Target;
	bool alreadyAttacking = false;
	for (auto unit : MainFight)
	{
		BWAPI::UnitCommandType currComm = unit->getLastCommand().getType();
		if (currComm == BWAPI::UnitCommandTypes::Attack_Move || currComm == BWAPI::UnitCommandTypes::Attack_Unit)
			alreadyAttacking = true;
	}
	if (MainFight.size() > 10 && !alreadyAttacking)
	{
		for (BWAPI::TilePosition starts : BWAPI::Broodwar->getStartLocations())
		{
			if (starts != myStart)
			{
				//std::cout << starts << std::endl;
				Target = BWAPI::Position(starts);
				//std::cout << Target << std::endl;
			}
		}
		MainFight.attack(Target);
		std::cout << "Attacking with units!" << std::endl;
	}
}

void SUnitManager::PlaceUnitCreated(BWAPI::Unit unit)
{
	// Based on the situation and need - place in different unitset

	switch (unit->getType())
	{
	case BWAPI::UnitTypes::Zerg_Zergling:
		if (Scouting.size() < 1)
			Scouting.insert(unit);
		else
			MainFight.insert(unit);
		return;
	case BWAPI::UnitTypes::Zerg_Drone:
		Workers.insert(unit);
	}
}

BWAPI::Unit SUnitManager::GetWorkerNearPosition(BWAPI::Position)
{
	return BWAPI::Unit();
}

BWAPI::Unit SUnitManager::GetScout()
{
	return BWAPI::Unit();
}

void SUnitManager::RemoveUnitFromUnitsets(BWAPI::Unit unit)
{
	if (Scouting.contains(unit))
		Scouting.erase(unit);

	if (MainFight.contains(unit))
		MainFight.erase(unit);
	
	if (Workers.contains(unit))
		Workers.erase(unit);
	
	if (Harass.contains(unit))
		Harass.erase(unit);
}
