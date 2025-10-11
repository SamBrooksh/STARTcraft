#pragma once

#include <BWAPI.h>
#include <queue>
#include <vector>
#include <utility>
#include <string>
#include "Tools.h"


typedef std::pair<BWAPI::UnitType, int> UnitPlan;	// Int is Timing? Priority for sure
struct UnitPlanComp {
	bool operator()(const UnitPlan& a, const UnitPlan& b) const noexcept {
		if (a.second != b.second) {
			return a.second > b.second;
		}
		return a.second < b.second;	//Should probably make this to do expensive units first?
	}
};

struct BuildingStage {
	BWAPI::Unit worker;
	BWAPI::TilePosition buildPos;
	BWAPI::UnitType toBuild;
	int frameIssued;
	int unitId;
};

class SBuildManager {
	std::priority_queue<UnitPlan, std::vector<UnitPlan>, UnitPlanComp> pq_planned;	//Eventually will make my own priority_queue with additional functionality
	int spentMins = 0;	// Have these be incremented when it occurs (easy for creating of units) but for structures when the structure gets thrown down
	int spentGas = 0;
	std::vector<BuildingStage> v_buildingsToBeBuilt;
	// BuildOrder* Planner;	// Eventually have a BuildOrder
	// Need probably a read from file to add to planned



public:
	//Just starter stuff - eventually something like this would be 
	void AddPlan(const UnitPlan);
	void SampleStart();
	void AddExpand();
	void AddZerglings();
	void AddOverlord();

	void BuildNext();
	void DrawBuildOrder();
	void StructureStarted(BWAPI::Unit);
	void WorkerKilled(BWAPI::Unit);
	void StructureDestroyed();	// If a tech structure is destroyed - need to know
};