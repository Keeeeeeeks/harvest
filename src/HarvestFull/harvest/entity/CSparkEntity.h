// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: only what recovered units use is declared.

#ifndef HARVEST_ENTITY_CSPARKENTITY_H
#define HARVEST_ENTITY_CSPARKENTITY_H

#include "CHarvestEntity.h"

namespace harvest {
namespace entity {

//! A spark of energy travelling between buildings.
class CSparkEntity : public CEntity
{
public:
    CSparkEntity(float x, float y, int, int);
    virtual ~CSparkEntity();

    virtual void writeEntityData(ox::io::IWriteFile* file);
    virtual void readEntityData(ox::io::IReadFile* file, int version);
    virtual int updateLogic(float frameDelta);
    virtual void render(const ox::core::CPosition2d<float>& camera, const ox::core::CRect<int>& viewPort);
    virtual void renderGroundLayer(const ox::core::CPosition2d<float>& camera,
        const ox::core::CRect<int>& viewPort);

private:
    // The layout is not recovered yet; this keeps the Linux amd64 object size (0x48).
    unsigned char Unrecovered[0x48 - sizeof(CEntity)];
};

} // end namespace entity
} // end namespace harvest

#endif
