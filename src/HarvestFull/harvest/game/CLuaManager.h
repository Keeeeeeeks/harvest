// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: only what recovered units use is declared.

#ifndef HARVEST_GAME_CLUAMANAGER_H
#define HARVEST_GAME_CLUAMANAGER_H

namespace harvest {
namespace entity {
class CBuildingEntity;
} // end namespace entity

namespace game {

//! Runs the Lua scripts of scenarios and the creative mode.
class CLuaManager
{
public:
    //! Tells the scripts that a building sent out a spark.
    void hookEnergySparkCreated(int sparkId, entity::CBuildingEntity* building);
    //! Tells the scripts that a construction site has become a building.
    void hookBuildingConstructed(entity::CBuildingEntity* building);
};

extern CLuaManager* gp_luaManager;

} // end namespace game
} // end namespace harvest

#endif
