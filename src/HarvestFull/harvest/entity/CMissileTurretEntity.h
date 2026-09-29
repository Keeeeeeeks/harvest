// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: only what recovered units use is declared.

#ifndef HARVEST_ENTITY_CMISSILETURRETENTITY_H
#define HARVEST_ENTITY_CMISSILETURRETENTITY_H

#include "CBuildingEntity.h"

namespace harvest {
namespace entity {

//! A turret that fires missiles; entity types 8, 13 and 14.
class CMissileTurretEntity : public CBuildingEntity
{
public:
    CMissileTurretEntity(int type, float x, float y);
    virtual ~CMissileTurretEntity();

    virtual void writeEntityData(ox::io::IWriteFile* file);
    virtual void readEntityData(ox::io::IReadFile* file, int version);
    virtual int updateLogic(float frameDelta);
    virtual void render(const ox::core::CPosition2d<float>& camera, const ox::core::CRect<int>& viewPort);
    virtual void renderGroundLayer(const ox::core::CPosition2d<float>& camera,
        const ox::core::CRect<int>& viewPort);

private:
    // The layout is not recovered yet; this keeps the Linux amd64 object size (0x78).
    unsigned char Unrecovered[0x78 - sizeof(CBuildingEntity)];
};

//! A missile fired by a missile turret.
class CMissileEntity : public CEntity
{
public:
    CMissileEntity(float x, float y, const ox::core::CPosition2d<float>&, int, int, int);
    virtual ~CMissileEntity();

    virtual void writeEntityData(ox::io::IWriteFile* file);
    virtual void readEntityData(ox::io::IReadFile* file, int version);
    virtual int updateLogic(float frameDelta);
    virtual void render(const ox::core::CPosition2d<float>& camera, const ox::core::CRect<int>& viewPort);
    virtual void renderGroundLayer(const ox::core::CPosition2d<float>& camera,
        const ox::core::CRect<int>& viewPort);

private:
    // The layout is not recovered yet; this keeps the Linux amd64 object size (0x68).
    unsigned char Unrecovered[0x68 - sizeof(CEntity)];
};

//! A blast fired by a tempest turret.
class CTempestBlastEntity : public CEntity
{
public:
    CTempestBlastEntity(const ox::core::CVector3d<float>& position, const ox::core::CVector3d<float>&, int, int);
    virtual ~CTempestBlastEntity();

    virtual void writeEntityData(ox::io::IWriteFile* file);
    virtual void readEntityData(ox::io::IReadFile* file, int version);
    virtual int updateLogic(float frameDelta);
    virtual void render(const ox::core::CPosition2d<float>& camera, const ox::core::CRect<int>& viewPort);
    virtual void renderGroundLayer(const ox::core::CPosition2d<float>& camera,
        const ox::core::CRect<int>& viewPort);

private:
    // The layout is not recovered yet; this keeps the Linux amd64 object size (0x68).
    unsigned char Unrecovered[0x68 - sizeof(CEntity)];
};

} // end namespace entity
} // end namespace harvest

#endif
