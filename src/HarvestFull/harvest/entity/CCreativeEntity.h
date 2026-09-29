// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: only what recovered units use is declared.

#ifndef HARVEST_ENTITY_CCREATIVEENTITY_H
#define HARVEST_ENTITY_CCREATIVEENTITY_H

#include "CBuildingEntity.h"

namespace harvest {
namespace entity {

//! A building of the creative game mode.
class CCreativeEntity : public CBuildingEntity
{
public:
    CCreativeEntity(float x, float y, const char*);
    virtual ~CCreativeEntity();

    virtual void writeEntityData(ox::io::IWriteFile* file);
    virtual void readEntityData(ox::io::IReadFile* file, int version);
    virtual int updateLogic(float frameDelta);
    virtual void render(const ox::core::CPosition2d<float>& camera, const ox::core::CRect<int>& viewPort);
    virtual void renderGroundLayer(const ox::core::CPosition2d<float>& camera,
        const ox::core::CRect<int>& viewPort);

private:
    // The layout is not recovered yet; this keeps the Linux amd64 object size (0xb0).
    unsigned char Unrecovered[0xb0 - sizeof(CBuildingEntity)];
};

} // end namespace entity
} // end namespace harvest

#endif
