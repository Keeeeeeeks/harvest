// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: only what recovered units use is declared.

#ifndef HARVEST_ENTITY_CALIENENTITY_H
#define HARVEST_ENTITY_CALIENENTITY_H

#include "CHarvestEntity.h"

namespace harvest {
namespace entity {

//! Localization keys of the alien types; the last five are unused placeholders.
static const wchar_t* const ALIEN_KEY_NAMES[] =
{
    L"aliennames:default",
    L"aliennames:shielder",
    L"aliennames:tiny",
    L"aliennames:summoner",
    L"aliennames:looker",
    L"aliennames:miner",
    L"aliennames:stealer",
    L"aliennames:magneto",
    L"aliennames:mega",
    L"aliennames:asdf",
    L"aliennames:asdf",
    L"aliennames:asdf",
    L"aliennames:asdf",
    L"aliennames:asdf"
};

//! An alien attacking the player.
class CAlienEntity : public CEntity
{
public:
    CAlienEntity(float x, float y, int alienType);
    virtual ~CAlienEntity();

    virtual void writeEntityData(ox::io::IWriteFile* file);
    virtual void readEntityData(ox::io::IReadFile* file, int version);
    virtual int updateLogic(float frameDelta);
    virtual void render(const ox::core::CPosition2d<float>& camera, const ox::core::CRect<int>& viewPort);
    virtual void renderGroundLayer(const ox::core::CPosition2d<float>& camera,
        const ox::core::CRect<int>& viewPort);

    //! Applies damage and knockback; returns true when the alien dies.
    bool dealDamage(float& damage, const ox::core::CPosition2d<float>& source, float force, int weapon);

    //! Movement destination at Linux amd64 offset 0x48; the accessor name is ours.
    const ox::core::CPosition2d<float>& getMovementTarget() const { return MovementTarget; }

    int getAlienType() const { return AlienType; }

private:
    // The layout is not recovered yet; this keeps the Linux amd64 object size (0x2f0).
    unsigned char Unrecovered[0x48 - 0x24];
    ox::core::CPosition2d<float> MovementTarget;
    unsigned char UnrecoveredMovement[0x60 - 0x50];
    int AlienType;
    unsigned char UnrecoveredTail[0x2f0 - 0x64];
};

} // end namespace entity
} // end namespace harvest

#endif
