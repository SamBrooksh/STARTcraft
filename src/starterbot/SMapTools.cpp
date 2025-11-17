#include "SMapTools.h"

void SMapTools::onStart()
{
    m_width = BWAPI::Broodwar->mapWidth();
    m_height = BWAPI::Broodwar->mapHeight();
    m_walkable = Grid<int>(m_width, m_height, 1);
    m_buildable = Grid<int>(m_width, m_height, 0);
    m_depotBuildable = Grid<int>(m_width, m_height, 0);
    m_lastSeen = Grid<int>(m_width, m_height, 0);
    m_tileType = Grid<char>(m_width, m_height, 0);

    // Set the boolean grid data from the Map
	// Change this to handle different uses
    for (int x(0); x < m_width; ++x)
    {
        for (int y(0); y < m_height; ++y)
        {
            m_buildable.set(x, y, canBuild(x, y));
            m_depotBuildable.set(x, y, canBuild(x, y));
            m_walkable.set(x, y, m_buildable.get(x, y) || canWalk(x, y));
        }
    }

    // set tiles that static resources are on as unbuildable
    for (auto& resource : BWAPI::Broodwar->getStaticNeutralUnits())
    {
        if (!resource->getType().isResourceContainer())
        {
            continue;
        }

        const int tileX = resource->getTilePosition().x;
        const int tileY = resource->getTilePosition().y;

        for (int x = tileX; x < tileX + resource->getType().tileWidth(); ++x)
        {
            for (int y = tileY; y < tileY + resource->getType().tileHeight(); ++y)
            {
                if (resource->getType().isMineralField()) { m_tileType.set(x, y, 'M'); }
                else { m_tileType.set(x, y, 'G'); }

                m_buildable.set(x, y, false);

                // depots can't be built within 3 tiles of any resource
                for (int rx = -3; rx <= 3; rx++)
                {
                    for (int ry = -3; ry <= 3; ry++)
                    {
                        if (!BWAPI::TilePosition(x + rx, y + ry).isValid())
                        {
                            continue;
                        }

                        m_depotBuildable.set(x + rx, y + ry, 0);
                    }
                }
            }
        }
    }

    // set the other tile types
    for (int x(0); x < m_width; ++x)
    {
        for (int y(0); y < m_height; ++y)
        {
            // if it has a type already it's a mineral or a gas
            if (m_tileType.get(x, y) != 0) { continue; }
            if (isBuildable(x, y) && !isDepotBuildableTile(x, y)) { m_tileType.set(x, y, 'D'); continue; }
            if (isBuildable(x, y)) { m_tileType.set(x, y, 'B'); continue; }
            if (isWalkable(x, y)) { m_tileType.set(x, y, 'W'); continue; }
            if (!isWalkable(x, y)) { m_tileType.set(x, y, 'U'); continue; }
        }
    }
}

bool SMapTools::validBaseLocation(BWAPI::TilePosition p)
{
    for (int w = 0; w < 4; ++w)
    {
        for (int h = 0; h < 3; ++h)
        {
            if (isDepotBuildableTile(p.x + w, p.y + h))
                return false;
        }
    }
    return true;
}

BWAPI::TilePosition SMapTools::getBaseLocation(BWAPI::Unitset& resources)
{
	// BFS will probably be the best here... maybe A* eventually
	std::queue<BWAPI::TilePosition> q;
	std::map<BWAPI::TilePosition, bool> visited;
	BWAPI::Unitset emptyMinerals;
	// remove minerals that are less than 100 minerals
	// /*
	std::for_each(resources.begin(), resources.end(), [](BWAPI::Unit u) { std::cout << u->getID() << std::endl; });

	resources.erase_if([&](BWAPI::Unit u) {
		if (u->getResources() < 100)
		{
			emptyMinerals.insert(u);
			return true;
		}
		else {
			return false;
		}
		});

	std::cout << "After\n";
	std::for_each(resources.begin(), resources.end(), [](BWAPI::Unit u) { std::cout << u->getID() << std::endl; });
	std::cout << "Empty Resources:\n";
	std::for_each(emptyMinerals.begin(), emptyMinerals.end(), [](BWAPI::Unit u) { std::cout << u->getID() << std::endl; });

	//*/

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

void SMapTools::getAllBasePositions()
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

void SMapTools::drawBases()
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

void SMapTools::displayMinPositions()
{
	for (auto min : BWAPI::Broodwar->getStaticMinerals())
	{
		int group = min->getResourceGroup();	//Gives the "id" of the expansion, not the optimal base location
		BWAPI::Broodwar->drawTextMap(min->getPosition(), "G: %d", group);
	}
}
