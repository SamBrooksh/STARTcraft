#include "SScoutManager.h"

bool isWalkableBetween(BWAPI::Position a, BWAPI::Position b)
{
	return BWAPI::Broodwar->getRegionAt(a)->getRegionGroupID() == BWAPI::Broodwar->getRegionAt(b)->getRegionGroupID();
	//BWAPI::WalkPosition()
	//BWAPI::Broodwar->isWalkable()
}

bool isVisitedAndValid(std::map<BWAPI::TilePosition, bool>& visit, BWAPI::TilePosition place, int xChange, int yChange, BWAPI::TilePosition startedFrom)
{
	if (!BWAPI::TilePosition(place.x + xChange, place.y + yChange).isValid())
		return false;
	

	if (!isWalkableBetween(BWAPI::Position(place.x * 32, place.y * 32), BWAPI::Position(startedFrom.x * 32, startedFrom.y * 32)))
		return false;

	return visit.count(BWAPI::TilePosition(place.x + xChange, place.y + yChange)) == 0;
}

bool isDefault(BWAPI::TilePosition p)
{
	if (p.x == 0 && p.y == 0)
		return true;
	//std::cout << "Not Defaulted: " << p << std::endl;
	return false;
}

bool SScoutManager::validBaseLocation(BWAPI::TilePosition p)
{
	for (int w = 0; w < 4; ++w)
	{
		for (int h = 0; h < 3; ++h)
		{
			if (!m_tools->isDepotBuildableTile(p.x + w, p.y + h))
				return false;
		}
	}
	return true;
}

BWAPI::TilePosition SScoutManager::getBaseLocation(BWAPI::Unitset &resources)
{
	// BFS will probably be the best here... maybe A* eventually
	std::queue<BWAPI::TilePosition> q;
	std::map<BWAPI::TilePosition, bool> visited;

	// check for a refinery nearby
	BWAPI::Unit ref = resources.getClosestUnit(BWAPI::Filter::GetType == BWAPI::UnitTypes::Resource_Vespene_Geyser, 500);
	if (BWAPI::Broodwar->getGeysers().contains(ref))
	{
		resources.insert(ref);
	}

	// get the two furthest away minerals
	BWAPI::TilePosition a, b;
	for (auto min : resources)
	{
		//std::cout << "Resource Tile Position: " << min->getTilePosition() << std::endl;
		if (isDefault(a))
		{
			a = min->getTilePosition();
		}
		else if (isDefault(b))
		{
			b = min->getTilePosition();
		}
		else if (a.getDistance(b) < a.getDistance(min->getTilePosition()))
		{
			b = min->getTilePosition();
		}
		else if (b.getDistance(a) < b.getDistance(min->getTilePosition()))
		{
			a = min->getTilePosition();
		}
	}

	//std::cout << "final a: " << a << " : final b:" << b << std::endl;

	// Should be the two furthest now
	// Get midpoint/mid tile and search in a circle surrounding it for now (should technically be able to go faster with two/four potential starting points) - maybe not
	BWAPI::TilePosition start = BWAPI::TilePosition((BWAPI::Position(a) + BWAPI::Position(b)) / 2);
	q.push(start);
	visited[start] = true;
	int count = 0;
	//std::cout << "Starting position: " << q.front() << std::endl;
	while (!q.empty() && count < 200)
	{
		//std::cout << "In loop. q size: " << q.size() << std::endl;
		BWAPI::TilePosition checking = q.front();
		CheckedSquares.push_back(q.front());
		q.pop();
		visited[checking] = true;
		//std::cout << "Checking: " << checking << std::endl;
		if (validBaseLocation(checking))
		{
			// May need to prevent leaks...
			return checking;
		}

		// Need to check +1 0, -1 0, 0 +1, 0 -1
		if (isVisitedAndValid(visited, checking, 1, 0, start))
		{
			BWAPI::TilePosition newPos = BWAPI::TilePosition(checking.x + 1, checking.y);
			q.push(newPos);
			visited[newPos] = true;
			//std::cout << "x + 1 pushed" << std::endl;
		}
		if (isVisitedAndValid(visited, checking, -1, 0, start))
		{
			BWAPI::TilePosition newPos = BWAPI::TilePosition(checking.x - 1, checking.y);
			q.push(newPos);
			visited[newPos] = true;
			//std::cout << "x - 1 pushed" << std::endl;
		}
		if (isVisitedAndValid(visited, checking, 0, 1, start))
		{
			BWAPI::TilePosition newPos = BWAPI::TilePosition(checking.x, checking.y + 1);
			q.push(newPos);
			visited[newPos] = true;
			//std::cout << "y + 1 pushed" << std::endl;
		}
		if (isVisitedAndValid(visited, checking, 0, -1, start))
		{
			BWAPI::TilePosition newPos = BWAPI::TilePosition(checking.x, checking.y - 1);
			q.push(newPos);
			visited[newPos] = true;
			//std::cout << "y - 1 pushed" << std::endl;
		}
		++count;
	}
	std::cout << "Tried to find hatch, but became empty. Count: " << count << std::endl;
	//std::cin >> count;
	return BWAPI::TilePositions::Invalid;
}

void SScoutManager::getAllBasePositions()
{
	std::vector<BWAPI::Unitset> mineralGroup;
	mineralGroup.reserve(250);
	for (auto& min : BWAPI::Broodwar->getStaticMinerals())
	{
		int group = min->getResourceGroup();	//Gives the "id" of the expansion, not the optimal base location
		//std::cout << "Mineral Group: " << group << ". Current Size: " << mineralGroup.size() << std::endl;
		if (mineralGroup.size() <= group + 1)
		{
			for (size_t i = mineralGroup.size(); i <= group; i++)
			{
				mineralGroup.push_back(BWAPI::Unitset());
				//std::cout << "Pushed a group" << std::endl;
			}
		}
		mineralGroup.at(group).insert(min);
		// std::cout << "Mineral Group (group) size now: " << mineralGroup.at(group).size() << std::endl;
	}
	//std::cout << "MineralGroup Size: " << mineralGroup.size() << std::endl;
	// All minerals now in each group
	// Go through all groups and find the closest position to place a base
	bool first = false;
	for (auto& g : mineralGroup)
	{
		// May want to confirm that this is a base and not blocking minerals somehow...
		if (!first)
		{
			first = true;
			continue;
		}
		//std::cout << "Mineral Group size now: " << g.size() << std::endl;
		allBaseLocations.push_back(getBaseLocation(g));
	}
}

void SScoutManager::displayMinPositions()
{
	for (auto min : BWAPI::Broodwar->getStaticMinerals())
	{
		int group = min->getResourceGroup();	//Gives the "id" of the expansion, not the optimal base location
		BWAPI::Broodwar->drawTextMap(min->getPosition(), "G: %d", group);
	}
}

void SScoutManager::drawBases()
{
	BWAPI::UnitType base = BWAPI::UnitTypes::Zerg_Hatchery;
	
	for (auto& pos : allBaseLocations)
	{
		BWAPI::Broodwar->drawBoxMap(BWAPI::Position(pos.x * 32, pos.y * 32), BWAPI::Position(pos.x * 32 + base.tileWidth() * 32, pos.y * 32 + base.tileHeight() * 32), BWAPI::Colors::Purple, false);
	}

	for (auto& pos : CheckedSquares)
	{
		const int padding = 2;
		const int px = pos.x * 32 + padding;
		const int py = pos.y * 32 + padding;
		const int d = 32 - 2 * padding;
		const BWAPI::Color color = BWAPI::Colors::Cyan;
		BWAPI::Broodwar->drawLineMap(px, py, px + d, py, color);
		BWAPI::Broodwar->drawLineMap(px + d, py, px + d, py + d, color);
		BWAPI::Broodwar->drawLineMap(px + d, py + d, px, py + d, color);
		BWAPI::Broodwar->drawLineMap(px, py + d, px, py, color);
	}
}

void SScoutManager::onStart()
{
	getAllBasePositions();
}

void SScoutManager::onFrame()
{
	drawBases();
}
