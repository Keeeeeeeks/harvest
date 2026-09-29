// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: only what recovered units use is declared.

#ifndef HARVEST_ENTITY_CENTITYMANAGER_H
#define HARVEST_ENTITY_CENTITYMANAGER_H

#include "ox/core/CVector3d.h"
#include "ox/entity/COxEntityManager.h"
#include <list>

namespace harvest {
namespace entity {

class CEntity;

//! The game's entity manager. It also sorts entities into a grid of cells for range searches.
class CEntityManager : public ox::entity::COxEntityManager
{
public:
    bool hasBuildingListChanged() const;

    //! The grid cell of a world coordinate, clamped to the grid.
    int calculateGridCoordinateClamp(float coordinate);
    void addGridEntity(CEntity* entity, int searchLayer);
    void removeGridEntity(CEntity* entity, const ox::core::CVector3d<float>& position, int searchLayer);

private:
    // The layout is not recovered yet; this keeps Grid at its Linux amd64 offset (0xa8).
    unsigned char Unrecovered[0xa8 - sizeof(ox::entity::COxEntityManager)];

public:
    //! The entities in each grid cell, indexed y * 18 + x.
    std::list<CEntity*> Grid[18 * 18];
};

extern CEntityManager* gp_entityManager;

} // end namespace entity
} // end namespace harvest

#endif
