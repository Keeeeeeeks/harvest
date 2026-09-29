// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

// The object has an iostream static initializer, set up before the game headers' statics; the
// header that pulled it in is not identified yet. The world grid offset is initialized before
// ENERGY_PROGRESS_COLOR, so CWorld.h comes first.
#include <iostream>
#include "harvest/game/CWorld.h"
#include "harvest/entity/CBuildingEntity.h"
#include "harvest/CHarvestFullMain.h"
#include "harvest/entity/CBuildableItems.h"
#include "harvest/entity/CCreativeEntity.h"
#include "harvest/game/CLuaManager.h"

namespace harvest {
namespace entity {

const char CBuildingLuaInfo::className[] = "CBuildingLuaInfo";

Lunar<CBuildingLuaInfo>::RegType CBuildingLuaInfo::methods[] =
{
    { "getId", &CBuildingLuaInfo::getId },
    { "getPosition", &CBuildingLuaInfo::getPosition },
    { "getBuildingType", &CBuildingLuaInfo::getBuildingType },
    { "getDeathParticle", &CBuildingLuaInfo::getDeathParticle },
    { "remove", &CBuildingLuaInfo::remove },
    { 0, 0 }
};

CBuildingEntity::CBuildingEntity(int id, int type, float x, float y)
    : CEntity(id, type, x, y)
{
    if (game::gp_luaManager || g_gameMode == 4)
    {
        LuaInfo = new CBuildingLuaInfo();
        LuaInfo->setEntity(this);
    }
    else
        LuaInfo = 0;
}

CBuildingEntity::~CBuildingEntity()
{
    if (LuaInfo)
        delete LuaInfo;
}

const char* CBuildingEntity::getDeathParticleName(int entityType)
{
    switch (entityType)
    {
    case 0:
        return "BuidlingExplosionSparkProducer";
    case 1:
        return "BuildingExplosionSparkMover";
    case 4:
        return "BuidlingExplosionMiner";
    case 7:
        return "BuildingExplosionDefenseTower";
    case 8:
        return "BuildingExplosionMissile";
    case 13:
        return "BuildingExplosionEagle";
    case 14:
        return "BuildingExplosionTempest";
    }

    return "BuildingExplosionSmall";
}

const char* CBuildingEntity::getBuildingType()
{
    if (Type == 3)
        return "CONSTRUCTIONSITE";
    if (Type == 16)
        return ((CCreativeEntity*)this)->getBuildingId();
    if (!gp_buildableItems)
        return "(null)";
    return gp_buildableItems->getEntityId(gp_buildableItems->getIndexForEntityType(Type));
}

CBuildingLuaInfo::CBuildingLuaInfo(lua_State* L)
    : Entity(0)
{
}

CBuildingLuaInfo::~CBuildingLuaInfo()
{
}

int CBuildingLuaInfo::getId(lua_State* L)
{
    lua_pushnumber(L, Entity->getId());
    return 1;
}

int CBuildingLuaInfo::getPosition(lua_State* L)
{
    lua_pushnumber(L, Entity->getPosition().X);
    lua_pushnumber(L, Entity->getPosition().Y);
    return 2;
}

int CBuildingLuaInfo::getBuildingType(lua_State* L)
{
    lua_pushstring(L, Entity->getBuildingType());
    return 1;
}

int CBuildingLuaInfo::getDeathParticle(lua_State* L)
{
    lua_pushstring(L, CBuildingEntity::getDeathParticleName(Entity->getEntityType()));
    return 1;
}

int CBuildingLuaInfo::remove(lua_State* L)
{
    Entity->killEntity();
    return 0;
}

} // end namespace entity
} // end namespace harvest
