// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: only what recovered units use is declared.

#ifndef HARVEST_ENTITY_CENTITYMANAGER_H
#define HARVEST_ENTITY_CENTITYMANAGER_H

#include "ox/core/CPosition2d.h"
#include "ox/core/CVector3d.h"
#include "ox/entity/COxEntityManager.h"
#include "ox/video/ISpriteAnimationState.h"
#include "ox/video/SColor.h"
#include <list>

namespace harvest {
namespace entity {

class CEntity;

//! A beam drawn from Start to End with sprites at both ends, above all entities.
struct SEnergyBeam
{
    SEnergyBeam()
        : Width(5.0f), Color(0xffffffff), Beam(0), StartSprite(0), StartScale(1.0f), EndSprite(0),
          EndScale(1.0f)
    {
    }

    virtual ~SEnergyBeam()
    {
        if (Beam)
            Beam->remove();
        if (StartSprite)
            StartSprite->remove();
        if (EndSprite)
            EndSprite->remove();
    }

    float Width;
    ox::core::CPosition2d<float> Start;
    ox::core::CPosition2d<float> End;
    ox::video::SColor Color;
    //! Stretched from Start to End.
    ox::video::ISpriteAnimationState* Beam;
    ox::video::ISpriteAnimationState* StartSprite;
    float StartScale;
    ox::video::ISpriteAnimationState* EndSprite;
    float EndScale;
};

//! The game's entity manager. It also sorts entities into a grid of cells for range searches.
class CEntityManager : public ox::entity::COxEntityManager
{
public:
    bool hasBuildingListChanged() const;

    //! The grid cell of a world coordinate, clamped to the grid.
    int calculateGridCoordinateClamp(float coordinate);
    void addGridEntity(CEntity* entity, int searchLayer);
    void removeGridEntity(CEntity* entity, const ox::core::CVector3d<float>& position, int searchLayer);
    //! The grid cell index of a position.
    int calculateGridPosition(const ox::core::CVector3d<float>& position);
    CEntity* locateEntityInGrid(int id, int gridPosition, int layer);
    CEntity* findRandomEntityInRange(const ox::core::CPosition2d<float>& position, float squaredRange, int layer,
        int entityType);
    //! Queues a beam to be drawn above all entities this frame.
    void insertTopLevelEnergyBeam(SEnergyBeam* beam);

private:
    // The layout is not recovered yet; this keeps Grid at its Linux amd64 offset (0xa8).
    unsigned char Unrecovered[0xa8 - sizeof(ox::entity::COxEntityManager)];

public:
    //! The entities in each grid cell, indexed y * 18 + x.
    std::list<CEntity*> Grid[18 * 18];
};

extern CEntityManager* gp_entityManager;
//! The id the next spark gets.
extern int g_nextEntityId;

} // end namespace entity
} // end namespace harvest

#endif
