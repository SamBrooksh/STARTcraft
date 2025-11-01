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
	// Based on the situation and need - place in different unitset.. May just want to put them in unsorted here, and sort them later

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
		return;
	default:
		Unsorted.insert(unit);
	}
}

void SUnitManager::SortUnsorted()
{
	for (auto& unit : Unsorted)
	{
		switch (unit->getType())
		{
		case BWAPI::UnitTypes::Zerg_Drone:
			if (!Workers.contains(unit))
				Workers.insert(unit);
			//Find a good one to add to
			for (auto& rw : MiningTracker)	//Maybe should change where this is done
			{
				if (rw.Workers.size() < 2)
				{
					
					rw.Workers.insert(unit);
					std::cout << "Added "<< unit <<" to Mining Tracker ("<<rw.Resource<<"): " <<rw.Workers.size() << std::endl;
					Unsorted.erase(unit);
					if (rw.Workers.contains(unit) && !Unsorted.contains(unit))
					{
						std::cout << "Works as wanted" << std::endl;
					}
					break;
				}
			}
		}
	}
}

void SUnitManager::MicroWorkerMining(BWAPI::Unit worker, BWAPI::Unit resource)
{
	// Check the commands and then tell them to mine if doing nothing
	if (worker->isGatheringGas() || worker->isGatheringMinerals())
	{
		
	}
	else if (worker->isIdle())	//I want to eventually make it speed mining
	{
		worker->gather(resource);
		std::cout << "Ordered Mining" << std::endl;
	}
}

void SUnitManager::Micro()
{
	int BFrame = BWAPI::Broodwar->getFrameCount();
	int count = 0;
	for (auto rw : MiningTracker)
	{
		//Draw on the minerals?
		BWAPI::Broodwar->drawText(BWAPI::CoordinateType::Map, rw.Resource->getLeft()-15, rw.Resource->getBottom()-15, "R: %d", count);
		int wCount = 0;
		for (auto worker : rw.Workers)
		{
			//if (worker->getLastCommandFrame() >= BFrame) { continue; }	// Will probably need this in all micro stuff...
			BWAPI::Broodwar->drawText(BWAPI::CoordinateType::Map, worker->getLeft() - 45, worker->getBottom() - 30, "%d->%d", wCount, count);

			MicroWorkerMining(worker, rw.Resource);
			wCount += 1;
		}
		count += 1;
	}
}

void SUnitManager::listResourceWorkers()
{
	for (auto rw : MiningTracker)
	{
		std::cout << "Resource: "<< rw.Resource <<std::endl;
		for (auto w : rw.Workers)
			std::cout << "\tWorker:" << w << std::endl;
	}
}

bool SUnitManager::isBase(BWAPI::Unit u)
{
	BWAPI::UnitType curr = u->getType();
	if (u->exists())
	{
		switch (curr)
		{
		case BWAPI::UnitTypes::Zerg_Hatchery:
		case BWAPI::UnitTypes::Zerg_Lair:
		case BWAPI::UnitTypes::Zerg_Hive:
			return true;
		}
	}
	return false;
}

void SUnitManager::AddResourcesToMiningTracker()
{
	// Go through each base, check to see if there are any minerals not yet in MiningTracker
	for (auto& u : Hatcheries)
	{
		BWAPI::Unitset mins = BWAPI::Broodwar->getUnitsInRadius(u->getPosition(), 256, BWAPI::Filter::IsMineralField);	//This could be way to large of a search...
		std::cout << "Resources Found: " << mins.size() << std::endl;
		for (auto& min : mins)
		{
			if (!ResourceInMiningTracker(min))
			{
				MiningTracker.push_back(ResourceWorkers(min, BWAPI::Unitset()));
				std::cout << "Added resource to Mining Tracker" << std::endl;
			}
		}
	}
}

bool SUnitManager::ResourceInMiningTracker(BWAPI::Unit Resource)
{
	return ResourceIndexInMining(Resource) >= 0;
}

int SUnitManager::ResourceIndexInMining(BWAPI::Unit Resource)
{
	for (int i = 0; i < MiningTracker.size(); ++i)
	{
		if (MiningTracker.at(i).Resource == Resource)
			return i;
	}
	//Not found
	return -1;
}

void SUnitManager::ResourceLost(BWAPI::Unit FieldLost)
{
	//Confirm the resources?
	int index = ResourceIndexInMining(FieldLost);
	if (index != -1)
	{
		// Add any workers that were designated to the Unsorted field
		for (auto& worker : MiningTracker.at(index).Workers)
			Unsorted.insert(worker);
		
		MiningTracker.erase(MiningTracker.begin() + index);	// I hope this doesn't leak mem
		std::cout << "ResourceLost" << std::endl;
	}
	else {
		//Should mark error of some way...
	}
}

void SUnitManager::onStart()
{
	for (auto& unit : BWAPI::Broodwar->self()->getUnits())
	{
		switch (unit->getType())
		{
		case BWAPI::UnitTypes::Zerg_Drone:
		case BWAPI::UnitTypes::Zerg_Overlord:
			Unsorted.insert(unit);
			break;
		case BWAPI::UnitTypes::Zerg_Hatchery:
			Hatcheries.insert(unit);
			std::cout << "Added Hatch" << std::endl;
			break;
		}
	}
	AddResourcesToMiningTracker();
}

void SUnitManager::onFrame()
{
	Micro();
	SortUnsorted();	//Maybe do this every 100 frames or something
	if (BWAPI::Broodwar->getFrameCount() % 1000 == 0)
		listResourceWorkers();
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
	
	for (auto rw : MiningTracker)
	{
		if (rw.Workers.contains(unit))
			rw.Workers.erase(unit);
		else if (rw.Resource == unit)
		{
			ResourceLost(unit);
		}
	}
	std::cout << "Removed Unit from UnitSet" << std::endl;
}
