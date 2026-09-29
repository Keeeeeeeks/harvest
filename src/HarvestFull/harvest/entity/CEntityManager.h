// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: only what recovered units use is declared.

#ifndef HARVEST_ENTITY_CENTITYMANAGER_H
#define HARVEST_ENTITY_CENTITYMANAGER_H

#include "ox/entity/COxEntityManager.h"

namespace harvest {
namespace entity {

class CEntityManager : public ox::entity::COxEntityManager
{
};

extern CEntityManager* gp_entityManager;

} // end namespace entity
} // end namespace harvest

#endif
