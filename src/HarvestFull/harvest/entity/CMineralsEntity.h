// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: only what recovered units use is declared.

#ifndef HARVEST_ENTITY_CMINERALSENTITY_H
#define HARVEST_ENTITY_CMINERALSENTITY_H

#include "CHarvestEntity.h"

namespace harvest {
namespace entity {

//! A mineral deposit.
class CMineralsEntity : public CEntity
{
public:
    CMineralsEntity(float x, float y, int);
    virtual ~CMineralsEntity();

    virtual void writeEntityData(ox::io::IWriteFile* file);
    virtual void readEntityData(ox::io::IReadFile* file, int version);
    virtual int updateLogic(float frameDelta);
    virtual void render(const ox::core::CPosition2d<float>& camera, const ox::core::CRect<int>& viewPort);
    virtual void renderGroundLayer(const ox::core::CPosition2d<float>& camera,
        const ox::core::CRect<int>& viewPort);

private:
    // The layout is not recovered yet; this keeps the Linux amd64 object size (0x50).
    unsigned char Unrecovered[0x50 - sizeof(CEntity)];
};

} // end namespace entity
} // end namespace harvest

#endif
