// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: only what recovered units use is declared.

#ifndef HARVEST_ENTITY_CDEFENSETOWERENTITY_H
#define HARVEST_ENTITY_CDEFENSETOWERENTITY_H

#include "CBuildingEntity.h"

namespace harvest {
namespace entity {

//! A tower that shoots at aliens.
class CDefenseTowerEntity : public CBuildingEntity
{
public:
    CDefenseTowerEntity(float x, float y);
    virtual ~CDefenseTowerEntity();

    virtual void writeEntityData(ox::io::IWriteFile* file);
    virtual void readEntityData(ox::io::IReadFile* file, int version);
    virtual int updateLogic(float frameDelta);
    virtual void render(const ox::core::CPosition2d<float>& camera, const ox::core::CRect<int>& viewPort);
    virtual void renderGroundLayer(const ox::core::CPosition2d<float>& camera,
        const ox::core::CRect<int>& viewPort);

private:
    // The layout is not recovered yet; this keeps the Linux amd64 object size (0x590).
    unsigned char Unrecovered[0x590 - sizeof(CBuildingEntity)];
};

} // end namespace entity
} // end namespace harvest

#endif
