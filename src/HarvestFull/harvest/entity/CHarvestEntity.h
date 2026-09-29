// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: only what recovered units use is declared.

#ifndef HARVEST_ENTITY_CHARVESTENTITY_H
#define HARVEST_ENTITY_CHARVESTENTITY_H

#include "ox/entity/COxEntity.h"
#include "ox/video/SColor.h"

namespace harvest {
namespace entity {

static const ox::video::SColor ENERGY_PROGRESS_COLOR(255, 224, 152, 76);

//! Base of the game's entities.
class CEntity : public ox::entity::COxEntity
{
};

} // end namespace entity
} // end namespace harvest

#endif
