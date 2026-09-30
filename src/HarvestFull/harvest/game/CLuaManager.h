// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: only what recovered units use is declared.

#ifndef HARVEST_GAME_CLUAMANAGER_H
#define HARVEST_GAME_CLUAMANAGER_H

#include "ox/core/CVector3d.h"
#include "ox/core/CString.h"
#include "ox/core/CRect.h"
#include "lua.hpp"

namespace harvest {
namespace entity {
class CBuildingEntity;
class CAlienEntity;
class CCreativeEntity;
} // end namespace entity

namespace game {

//! Runs the Lua scripts of scenarios and the creative mode.
class CLuaManager
{
public:
    void hookMapExpanded(float left, float top, float right, float bottom);
    const ox::core::CRect<float>& getMinimumWorldBorders();
    void hookCreativeInit(const ox::core::CString<char>& id, entity::CCreativeEntity* building);
    void hookCreativeUpdate(const ox::core::CString<char>& id, entity::CCreativeEntity* building, float frameDelta);
    //! Tells the scripts that a building sent out a spark.
    void hookAlienDeath(int alienType, const ox::core::CVector3d<float>& position);
    void hookBuildingDestroyed(const char* buildingType, float x, float y, entity::CAlienEntity* alien);
    void hookEnergySparkCreated(int sparkId, entity::CBuildingEntity* building);
    //! Tells the scripts that a construction site has become a building.
    void hookBuildingConstructed(entity::CBuildingEntity* building);
    void hookMissileLaunched(int missileType, entity::CBuildingEntity* turret, int targetId, float x, float y);
    void hookCreditsMined(entity::CBuildingEntity* miner, int mineralsId);
    void hookMinerOutOfMinerals(entity::CBuildingEntity* miner);
    //! A spark died at an overheated energy link.
    void hookEnergyLinkOverheated(entity::CBuildingEntity* link);
    void hookEnergyLinkCharging(entity::CBuildingEntity* link);
    //! An energy link finished charging and exploded at x, y.
    void hookEnergyLinkCharged(float x, float y);
};

extern CLuaManager* gp_luaManager;
extern lua_State* gp_luaState;
ox::core::CString<char> extractLuaPath(lua_State* L);

} // end namespace game
} // end namespace harvest

#endif
