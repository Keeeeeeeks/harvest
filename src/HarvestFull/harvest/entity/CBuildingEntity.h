// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: only what recovered units use is declared.

#ifndef HARVEST_ENTITY_CBUILDINGENTITY_H
#define HARVEST_ENTITY_CBUILDINGENTITY_H

#include "CHarvestEntity.h"

namespace harvest {
namespace entity {

//! Base of the player's buildings. It adds no virtual functions.
class CBuildingEntity : public CEntity
{
public:
    CBuildingEntity(int id, int type, float x, float y);
    virtual ~CBuildingEntity();
};

} // end namespace entity
} // end namespace harvest

#endif
