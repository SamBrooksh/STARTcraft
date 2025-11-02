#include "SScoutManager.h"

void SScoutManager::getAllBasePositions()
{
	for (auto min : BWAPI::Broodwar->getStaticMinerals())
	{
		min->getResourceGroup();	//Gives the "id" of the expansion, not the optimal base location
	}
}

void SScoutManager::onStart()
{
}
