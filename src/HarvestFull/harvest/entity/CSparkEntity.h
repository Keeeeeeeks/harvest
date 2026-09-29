// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#ifndef HARVEST_ENTITY_CSPARKENTITY_H
#define HARVEST_ENTITY_CSPARKENTITY_H

#include "CHarvestEntity.h"
#include "ox/entity/COxEntityManager.h"

namespace harvest {
namespace entity {

//! A spark of energy flying from building to building. Each building it reaches decides where it
//! goes next.
class CSparkEntity : public CEntity
{
public:
    CSparkEntity(float x, float y, int targetId, int sourceId);
    virtual ~CSparkEntity();

    virtual void writeEntityData(ox::io::IWriteFile* file);
    virtual void readEntityData(ox::io::IReadFile* file, int version);
    virtual void updateSprite(float frameDelta);
    virtual int updateLogic(float frameDelta);
    virtual void render(const ox::core::CPosition2d<float>& camera, const ox::core::CRect<int>& viewPort);
    virtual void renderGroundLayer(const ox::core::CPosition2d<float>& camera,
        const ox::core::CRect<int>& viewPort) {}

    //! The building the spark came from.
    int getSourceId() const;

private:
    ox::entity::COxEntity* findEntity(int id);

    ox::video::ISpriteAnimationState* Sprite;
    ox::entity::SEntityReference Target;
    int SourceId;
    //! The target's grid cell, or -1; it finds the target again when the buildings change.
    int GridPosition;
};

} // end namespace entity
} // end namespace harvest

#endif
