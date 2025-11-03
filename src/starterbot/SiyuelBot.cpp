#include "SiyuelBot.h"

void SiyuelBot::onStart()
{
    // Set our BWAPI options here    
    BWAPI::Broodwar->setLocalSpeed(10);
    BWAPI::Broodwar->setFrameSkip(0);
    
    b_buildManage.s_UnitManage = &s_unitManage;
    // Enable the flag that tells BWAPI top let users enter input while bot plays
    BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput);
    // Call MapTools OnStart
    m_mapTools.onStart();
    b_buildManage.SampleStart();
    s_unitManage.onStart();
}


void SiyuelBot::onFrame()
{
    if (printErrors)
    {
        showErrors();
    }
    if (displayDebug)
    {
        drawDebugInformation();
    }

    // Update our MapTools information
    m_mapTools.onFrame();
    
    b_buildManage.onFrame();
    // Draw unit health bars, which brood war unfortunately does not do
    Tools::DrawUnitHealthBars();
    
    s_unitManage.GatherAndAttack(); //Should change to a general one
    s_unitManage.onFrame();
}


// Send our idle workers to mine minerals so they don't just stand there    - Eventually don't use this
void SiyuelBot::sendIdleWorkersToMinerals()
{
    // Let's send all of our starting workers to the closest mineral to them
    // First we need to loop over all of the units that we (BWAPI::Broodwar->self()) own
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    for (auto& unit : myUnits)
    {
        // Check the unit type, if it is an idle worker, then we want to send it somewhere
        if (unit->getType().isWorker() && unit->isIdle())
        {
            // Get the closest mineral to this worker unit
            BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(unit, BWAPI::Broodwar->getMinerals());

            // If a valid mineral was found, right click it with the unit in order to start harvesting
            if (closestMineral) { unit->rightClick(closestMineral); }
        }
    }
}

void SiyuelBot::drawDebugInformation()
{
    //BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 10), "Hello, World!\n");
    Tools::DrawUnitCommands();
    Tools::DrawUnitBoundingBoxes();
    for (auto& unit : BWAPI::Broodwar->self()->getUnits())
    {
        BWAPI::Broodwar->drawText(BWAPI::CoordinateType::Map, (unit->getLeft() + unit->getRight()) / 2, (unit->getTop() + unit->getBottom()) / 2, "%d", unit->getID());
    }
}

// Called whenever the game ends and tells you if you won or not
void SiyuelBot::onEnd(bool isWinner)
{
    std::cout << "We " << (isWinner ? "won!" : "lost!") << "\n";
}

// Called whenever a unit is destroyed, with a pointer to the unit
void SiyuelBot::onUnitDestroy(BWAPI::Unit unit)
{
    s_unitManage.RemoveUnitFromUnitsets(unit);
    if (unit->getType().isWorker())
        b_buildManage.WorkerKilled(unit);
}

// Called whenever a unit is morphed, with a pointer to the unit
// Zerg units morph when they turn into other units
void SiyuelBot::onUnitMorph(BWAPI::Unit unit)
{
    if (unit->isMorphing())
        b_buildManage.StructureStarted(unit);
    
}

// Called whenever a text is sent to the game by a user
void SiyuelBot::onSendText(std::string text)
{
    if (text == "/map")
    {
        m_mapTools.toggleDraw();
    }
    else if (text == "/build")
    {
        b_buildManage.toggleDebug();
    }
    else if (text == "/micro")
    {
        s_unitManage.toggleDebug();
    }
    else if (text == "/errors")
    {
        printErrors = !printErrors;
    }
    else if (text == "/debug")
    {
        displayDebug = !displayDebug;
    }
}

void SiyuelBot::showErrors()
{
    BWAPI::Error e = BWAPI::Broodwar->getLastError();
    if (e != BWAPI::Errors::None)
    {
        std::cout << e << std::endl;
    }
}


// Called whenever a unit is created, with a pointer to the destroyed unit
// Units are created in buildings like barracks before they are visible, 
// so this will trigger when you issue the build command for most units
void SiyuelBot::onUnitCreate(BWAPI::Unit unit)
{
    
}

// Called whenever a unit finished construction, with a pointer to the unit
void SiyuelBot::onUnitComplete(BWAPI::Unit unit)
{
    s_unitManage.PlaceUnitCreated(unit);
}

// Called whenever a unit appears, with a pointer to the destroyed unit
// This is usually triggered when units appear from fog of war and become visible
void SiyuelBot::onUnitShow(BWAPI::Unit unit)
{

}

// Called whenever a unit gets hidden, with a pointer to the destroyed unit
// This is usually triggered when units enter the fog of war and are no longer visible
void SiyuelBot::onUnitHide(BWAPI::Unit unit)
{

}

// Called whenever a unit switches player control
// This usually happens when a dark archon takes control of a unit
void SiyuelBot::onUnitRenegade(BWAPI::Unit unit)
{

}