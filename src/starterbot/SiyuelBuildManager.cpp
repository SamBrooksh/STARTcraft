#include "SiyuelBuildManager.h"

void SBuildManager::AddPlan(UnitPlan up)
{
	// Will probably slowly increase the id of the group - so this should probably have count of to place as well
	this->pq_planned.push(up);
}

void SBuildManager::SampleStart()
{
	for (unsigned int i = 0; i < 6; i++)
	{
		this->AddPlan(UnitPlan(BWAPI::UnitTypes::Zerg_Drone, 1+i));
	}
	this->AddPlan(UnitPlan(BWAPI::UnitTypes::Zerg_Spawning_Pool, 3));

}

void SBuildManager::AddExpand()
{
	this->AddPlan(UnitPlan(BWAPI::UnitTypes::Zerg_Hatchery, 4));
}

void SBuildManager::AddZerglings()
{
	for (unsigned int i = 0; i < 6; i++)
		this->AddPlan(UnitPlan(BWAPI::UnitTypes::Zerg_Zergling, 1));
}

void SBuildManager::AddOverlord()
{
	this->AddPlan(UnitPlan(BWAPI::UnitTypes::Zerg_Overlord, 0));
}

void SBuildManager::BuildNext()
{
	//If close to supply block
	const int unusedSupply = Tools::GetTotalSupply(true) - BWAPI::Broodwar->self()->supplyUsed();

	BWAPI::Broodwar->drawTextScreen(BWAPI::Position(300, 310), std::to_string(unusedSupply).c_str());
	// If we have a sufficient amount of supply, we don't need to do anything
	if (unusedSupply >= 2 || pq_planned.top().first == BWAPI::UnitTypes::Zerg_Overlord) // This should change probably depending on state of game (and if max supply)
	{ }	//This will probably change based off the state - As well as what is already in the queue
	else { 
		AddOverlord();
	//std::cout << pq_planned.top().first << " - " << BWAPI::UnitTypes::Zerg_Overlord << std::endl;
	}


	//Should select from hatcheries, but ignore for now
	if (pq_planned.size() == 0)
		AddZerglings();	//Should do more with the build
	//
	const BWAPI::UnitType ToTrain = pq_planned.top().first;

	if (ToTrain.mineralPrice() + spentMins > BWAPI::Broodwar->self()->gatheredMinerals() || ToTrain.gasPrice() + spentGas > BWAPI::Broodwar->self()->gatheredGas())
	{
		return;	//Will eventually need to retool this to check next one to train and the like - for now miss it
	}
	
	// Need to add this as a task probably to be done in case of errors
	if (ToTrain.isBuilding())
	{
		const bool startedBuilding = Tools::BuildBuilding(ToTrain);
		if (startedBuilding)
		{
			BWAPI::Broodwar->printf("Started Building %s", ToTrain.getName().c_str());
			pq_planned.pop();
			spentGas += ToTrain.gasPrice();
			spentMins += ToTrain.mineralPrice();
		}
	}
	else 
	{
		BWAPI::Unitset larva = BWAPI::Broodwar->self()->getUnits().getLarva();
		const bool succ = larva.morph(ToTrain);
		if (succ)
		{
			pq_planned.pop();
			spentGas += ToTrain.gasPrice();
			spentMins += ToTrain.mineralPrice();
		}
	}
}

void SBuildManager::DrawBuildOrder()
{
	std::string build;
	std::string spent;
	std::priority_queue<UnitPlan, std::vector<UnitPlan>, UnitPlanComp> display(pq_planned);
	for (; !display.empty(); display.pop())
	{
		build += display.top().first.getName() + ',' + std::to_string(display.top().second) + '\n';	//Not quite working yet
		//std::cout << build << std::endl;// << display.top().first.c_str() << display.top().second << std::endl;
	}
	//std::cout << std::endl;
	spent += "Minerals Spent: " + std::to_string(spentMins) + " - Minerals Gathered: " + std::to_string(BWAPI::Broodwar->self()->gatheredMinerals());
	spent += "\nGas Spent: " + std::to_string(spentGas) + " - Gas Gathered: " + std::to_string(BWAPI::Broodwar->self()->gatheredGas());

	BWAPI::Broodwar->drawTextScreen(BWAPI::Position(20, 20), build.c_str());
	BWAPI::Broodwar->drawTextScreen(BWAPI::Position(400, 20), spent.c_str());
}
