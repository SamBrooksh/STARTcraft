#pragma once

#include <BWAPI.h>
#include "SiyuelBuildManager.h"
#include "MapTools.h"
#include "SUnitManager.h"
#include "SScoutManager.h"
#include "algorithm"

class SiyuelBot {

	MapTools m_mapTools;
	SBuildManager b_buildManage;
	SUnitManager s_unitManage;
	SScoutManager s_scoutManage;

	bool printErrors = false;
	bool displayDebug = false;
	/*
	* Do Zerg AI for now
	* TODO: Build order manager
	* Building Placement (Based on per map?) - Able to adjust from scouting information
	* Unit controller (with different goals) - (Harass, Worker mining, Defending Harassment, scouting, attacking, tactics)
	* 
	* Income Manager - Used to track income (and probably should be used to infer opponents info)
	* 
	* Stage of Game - Need a State Machine to switch different AI choices I think
	* 
	* Build Order -> Not a Queue - Need timings as to when to pull worker to build structure (including time to arrive accounted for)
	* Need to be able to adjust - Currently probably a priority queue? - with the ability to erase everything in it
	* 
	* Scouting -> Want to track and estimate opponents economy (assume information)
	*	* So maybe want a concrete seen, and assumption that changes based off new information
	*/

public:
	void onStart();
	void onFrame();
	void onEnd(bool isWinner);
	void onUnitDestroy(BWAPI::Unit unit);
	void onUnitMorph(BWAPI::Unit unit);
	void onSendText(std::string text);
	void onUnitCreate(BWAPI::Unit unit);
	void onUnitComplete(BWAPI::Unit unit);
	void onUnitShow(BWAPI::Unit unit);
	void onUnitHide(BWAPI::Unit unit);
	void onUnitRenegade(BWAPI::Unit unit);
	void sendIdleWorkersToMinerals();
	void drawDebugInformation();
	void showErrors();
};