#pragma once

#include <BWAPI.h>
#include "Grid.hpp"
#include <vector>
#include <queue>

class PossibleDepotInfo {

    BWAPI::Unitset miningMinerals;  // the resources to have as the base
    BWAPI::Unitset toRemove;        // Used to track things needed to remove from beginning
    BWAPI::TilePosition idealTile;

public:
    PossibleDepotInfo() : miningMinerals(), toRemove(), idealTile() {}
    PossibleDepotInfo(BWAPI::Unitset mins, BWAPI::Unitset rem, BWAPI::TilePosition t) : miningMinerals(mins), toRemove(rem), idealTile(t) {}
};

class SMapTools {

    int m_width;
    int m_height;
    Grid<int> m_walkable;
    Grid<int> m_buildable;
    Grid<int> m_depotSoonBuildable;
    Grid<int> m_depotBuildable;
    Grid<int> m_lastSeen;
    Grid<char> m_tileType;
    std::vector<PossibleDepotInfo> m_bases;

    bool canBuild(int tileX, int tileY) const;
    bool canWalk(int tileX, int tileY) const;

public:
	void onStart();
    std::vector<PossibleDepotInfo> getAllBaseLocations();
    std::vector<PossibleDepotInfo> getAllIslandBases();
    std::vector<PossibleDepotInfo> getAllStartBlockedBases();
    std::vector<PossibleDepotInfo> getNonIslandBases();

    bool    isValidTile(int tileX, int tileY) const;
    bool    isValidTile(const BWAPI::TilePosition& tile) const;
    bool    isValidPosition(const BWAPI::Position& pos) const;
    bool    isPowered(int tileX, int tileY) const;
    bool    isExplored(int tileX, int tileY) const;
    bool    isExplored(const BWAPI::Position& pos) const;
    bool    isExplored(const BWAPI::TilePosition& pos) const;
    bool    isVisible(int tileX, int tileY) const;
    bool    isWalkable(int tileX, int tileY) const;
    bool    isWalkable(const BWAPI::TilePosition& tile) const;
    bool    isBuildable(int tileX, int tileY) const;
    bool    isBuildable(const BWAPI::TilePosition& tile) const;
    bool    isDepotBuildableTile(int tileX, int tileY) const;
    void    drawTile(int tileX, int tileY, const BWAPI::Color& color) const;

    bool validBaseLocation(BWAPI::TilePosition p);
    void displayMinPositions();
    void drawBases();
    void getAllBasePositions();
    BWAPI::TilePosition getBaseLocation(BWAPI::Unitset& resources);
};